#include <algorithm>
#include "AsyncConnection.h"

namespace crown
{
namespace network
{

AsyncConnection::AsyncConnection(Allocator& allocator) :
 	m_sent_msg(allocator),
 	m_received_msg(allocator),
 	m_pending_ack(allocator),
 	m_acked(allocator)
{
  
}

//-----------------------------------------------------------------------------
AsyncConnection::~AsyncConnection()
{
  
}

//-----------------------------------------------------------------------------
void AsyncConnection::init(const os::NetAddress addr, const int id)
{
	m_remote_address = addr;
	m_id = id;
	m_max_rate = 64000;
	m_outgoing_rate_time = 0;
	m_outgoing_rate_bytes = 0;
	m_incoming_rate_time = 0;
	m_incoming_rate_bytes = 0;
	m_incoming_recv_packets = 0.0f;
	m_incoming_dropped_packets = 0.0f;
	m_incoming_packet_loss_time = 0;
	m_outgoing_sent_packet = 0;
	m_remote_sequence = 0;
	m_local_sequence = 0;
	m_max_sequence = 0xFFFFFFFF;
	m_max_rtt = 1000;	//in milliseconds
	m_rtt = 0;		
	m_last_send_time = 0;
	m_last_data_bytes = 0;
	
	// open port
	m_socket.open(addr.get_port());
	assert(m_socket.is_open());
}

//-----------------------------------------------------------------------------
void AsyncConnection::reset_rate()
{
  	m_last_send_time = 0;
	m_last_data_bytes = 0;
	m_outgoing_rate_time = 0;
	m_outgoing_rate_bytes = 0;
	m_incoming_rate_time = 0;
	m_incoming_rate_bytes = 0;
}

//-----------------------------------------------------------------------------
void AsyncConnection::set_max_outgoing_rate(int rate)
{
	m_max_rate = rate;
}

//-----------------------------------------------------------------------------
int AsyncConnection::get_max_outgoing_rate()
{
	return m_max_rate;
}

//-----------------------------------------------------------------------------
os::NetAddress AsyncConnection::get_remote_address() const
{
	return m_remote_address;
}

//-----------------------------------------------------------------------------
int AsyncConnection::get_outgoing_rate() const
{
	return m_outgoing_rate_bytes;
}

//-----------------------------------------------------------------------------
int AsyncConnection::get_incoming_rate() const
{
	return m_incoming_rate_bytes;
}

//-----------------------------------------------------------------------------
float AsyncConnection::get_incoming_packet_loss() const
{
	if (m_incoming_recv_packets == 0 && m_incoming_dropped_packets == 0)
	{
		return 0.0f;
	}
	// return loss packet %
	return m_incoming_dropped_packets * 100 / (m_incoming_recv_packets + m_incoming_dropped_packets);
}

//-----------------------------------------------------------------------------
void AsyncConnection::send_message(BitMessage& msg, const uint32_t time)
{
	m_socket.send(m_remote_address, msg.get_data(), msg.get_size());
	_packet_sent(msg.get_size());
}

//-----------------------------------------------------------------------------
bool AsyncConnection::receive_message(BitMessage& msg, const uint32_t time)
{
	size_t size;
	
	m_socket.receive(m_remote_address, msg.get_data(), size);
	msg.set_size(size);
	msg.begin_reading();

	//TODO: check return value of receive
	BitMessage::Header header;

	msg.read_int32();	// read protocol id
	header.sequence = msg.read_int32();	// read sequence
	msg.read_int32();// read ack
	msg.read_int32();// read ack_bits
	header.size = msg.read_uint16();// read size
}

//-----------------------------------------------------------------------------
void AsyncConnection::clear_reliable_messages()
{
	m_sent_msg.clear();
	m_received_msg.clear();
}

//-----------------------------------------------------------------------------
bool AsyncConnection::ready_to_send(const int time) const
{
	// if max rate isn't set, send message
	if (!m_max_rate)
	{
		return true;
	}
	
	int delta_time;
	
	delta_time = time - m_last_send_time;
	if (delta_time > 1000)
	{
		return true;
	}
	// if last message wasn't sent, sent it!
	return ((m_last_data_bytes - ((delta_time * m_max_rate) / 1000)) <= 0);
}

//-----------------------------------------------------------------------------
bool AsyncConnection::process(const os::NetAddress from, int time, BitMessage &msg, int &sequence)
{

}

//-----------------------------------------------------------------------------
void AsyncConnection::_packet_sent(size_t size)
{
	bool seq_exists_in_sent_queue = false;
	bool seq_exists_in_pending_ack_queue = false;
  
	BitMessage::Header tmp;
	BitMessage::Header* h_ptr;
	
	tmp.sequence = m_local_sequence;

	// If local_sequence_number is already in sent_queue
	h_ptr = std::find(m_sent_msg.begin(), m_sent_msg.end(), tmp);
	if (h_ptr != m_sent_msg.end())
	{
		seq_exists_in_sent_queue = true;
	}
	// If local_sequence_number is already in pending_ack_queue
	h_ptr = std::find(m_pending_ack.begin(), m_pending_ack.end(), tmp);
	if(h_ptr != m_pending_ack.end())
	{
		seq_exists_in_pending_ack_queue = true;
	}	
	// Else
	assert(!seq_exists_in_sent_queue);
 	assert(!seq_exists_in_pending_ack_queue);
	
	// Creates Header for saving in queues
	BitMessage::Header header;
	header.sequence = m_local_sequence;
	header.time = 0.0f;
	header.size = size;
	// Push packet data in sent_queue
	m_sent_msg.push_back(header);
	// push packet data in pending_ack_queue
	m_pending_ack.push_back(header);
	// Increments sent packet
	m_outgoing_sent_packet++;
	// Increments local sequence
	m_local_sequence++;

	if (m_local_sequence > m_max_sequence)
	{
		m_local_sequence = 0;
	}
}

//-----------------------------------------------------------------------------
void AsyncConnection::_packet_received(uint16_t sequence, size_t size)
{
	BitMessage::Header tmp;
	BitMessage::Header* h_ptr;

	tmp.sequence = sequence;

	// Increment received packets
	m_incoming_recv_packets++;
	
	// If packet's sequence exists, return
	h_ptr = std::find(m_received_msg.begin(), m_received_msg.end(), tmp);
	if (h_ptr != m_received_msg.end())
	{
		return;
	}
	
	BitMessage::Header header;
	header.sequence = sequence;
	header.time = 0.0f;
	header.size = size;
	// Push packet data in received_queue
	m_received_msg.push_back(header);
	// update m_remote_sequence
	if (_sequence_more_recent(sequence, m_remote_sequence))
	{
		m_remote_sequence = sequence;
	}  
}

//-----------------------------------------------------------------------------
void AsyncConnection::_process_ack(uint16_t ack, int32_t ack_bits)
{
	if (m_pending_ack.empty())
	{
		return;
	}
	
	BitMessage::Header* i = m_pending_ack.begin();
	while (i != m_pending_ack.end())
	{
		bool acked = false;
		
		if (i->sequence == ack)
		{
			acked = true;
		}
		else if (!_sequence_more_recent(i->sequence, ack))
		{
			uint32_t bit_index = _bit_index_for_sequence(i->sequence, ack);
			if (bit_index <= 31)
			{
				acked = (ack_bits >> bit_index) & 1;
			}
		}

		if (acked)
		{
			m_rtt += (i->time - m_rtt) * 0.1f;
			
			m_acked.push_back(*i);
		}
		else
		{
			++i;
		}
	}  
	m_pending_ack.clear();
}


//-----------------------------------------------------------------------------
bool AsyncConnection::_sequence_more_recent(uint16_t s1, uint16_t s2)
{
	return ((s1 > s2) && (s1 - s2 <= m_max_sequence / 2)) || ((s2 > s1) && (s2 - s1<= m_max_sequence / 2 ));
}

//-----------------------------------------------------------------------------
int32_t AsyncConnection::_bit_index_for_sequence(uint16_t seq, uint16_t ack)
{
	assert(seq != ack);
	assert(!_sequence_more_recent(seq, ack));
	
	if (seq > ack)
	{
		assert(ack < 33);
		assert(seq <= m_max_sequence);
		return ack + (m_max_sequence - seq);
	}
	else
	{
		assert(ack >= 1);
		assert(seq <= ack - 1);
		return ack - 1 - seq;
	}  
}

//-----------------------------------------------------------------------------
int32_t AsyncConnection::_generate_ack_bits(uint16_t ack)
{
	int32_t ack_bits = 0;
	
	for (BitMessage::Header* i = m_received_msg.begin(); i != m_received_msg.end(); i++)
	{
		if (i->sequence == ack || _sequence_more_recent(i->sequence, ack))
		{
			break;
		}
		
        int32_t bit_index = _bit_index_for_sequence(i->sequence, ack);
		if (bit_index <= 31)
		{
			ack_bits |= 1 << bit_index;
		}
	}
	return ack_bits;
}

//-----------------------------------------------------------------------------
void AsyncConnection::_update_outgoing_rate(const uint32_t time, const size_t size)
{
	// update the outgoing rate control variables
	int delta_time;
	delta_time = time - m_last_send_time;
	if (delta_time > 1000) 
	{
		m_last_data_bytes = 0;
	}
	else 
	{
		m_last_data_bytes -= (delta_time * m_max_rate) / 1000;
		if ( m_last_data_bytes < 0 ) 
		{
			m_last_data_bytes = 0;
		}
	}
	
	m_last_data_bytes += size;
	m_last_send_time = time;  
	
	// update outgoing rate variables
	if (time - m_outgoing_rate_time > 1000) 
	{
		m_outgoing_rate_bytes -= m_outgoing_rate_bytes * (time - m_outgoing_rate_time - 1000) / 1000;
		if (m_outgoing_rate_bytes < 0) 
		{
			m_outgoing_rate_bytes = 0;
		}
	}
	
	m_outgoing_rate_time = time - 1000;
	m_outgoing_rate_bytes += size;
}

//-----------------------------------------------------------------------------
void AsyncConnection::_update_incoming_rate(const uint32_t time, const size_t size)
{
	// update incoming rate variables
	if (time - m_incoming_rate_time > 1000) 
	{
		m_incoming_rate_bytes -= m_incoming_rate_bytes * (time - m_incoming_rate_time - 1000) / 1000;
		if (m_incoming_rate_bytes < 0) 
		{
			m_incoming_rate_bytes = 0;
		}
	}
	m_incoming_rate_time = time - 1000;
	m_incoming_rate_bytes += size;  
}

//-----------------------------------------------------------------------------
void AsyncConnection::_update_packet_loss(const uint32_t time, const uint32_t num_recv, const uint32_t num_dropped)
{
	// update incoming packet loss variables
	if (time - m_incoming_packet_loss_time > 5000) 
	{
		float scale = (time - m_incoming_packet_loss_time - 5000) * (1.0f / 5000.0f);
		m_incoming_recv_packets -= m_incoming_recv_packets * scale;
		if (m_incoming_recv_packets < 0.0f) 
		{
			m_incoming_recv_packets = 0.0f;
		}
		m_incoming_dropped_packets -= m_incoming_dropped_packets * scale;
		if (m_incoming_dropped_packets < 0.0f) 
		{
			m_incoming_dropped_packets = 0.0f;
		}
	}
	m_incoming_packet_loss_time = time - 5000;
	m_incoming_recv_packets += num_recv;
	m_incoming_dropped_packets += num_dropped;
}

} // namespace network
} // namespace crown
