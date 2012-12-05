#include <algorithm>
#include "AsyncConnection.h"

namespace crown
{
namespace network
{

AsyncConnection::AsyncConnection(Allocator& allocator) :
	m_remote_address(0, 0, 0, 0, 0),
	m_mode(NONE),
	m_state(DISCONNECTED),
	m_max_rate(MAX_RATE),
	m_outgoing_rate_time(0),
	m_outgoing_rate_bytes(0),
	m_incoming_rate_time(0),
	m_incoming_rate_bytes(0),
	m_incoming_recv_packets(0.0f),
	m_incoming_dropped_packets(0.0f),
	m_incoming_packet_loss_time(0),
	m_outgoing_sent_packet(0),
	m_local_sequence(0),
	m_remote_sequence(0),
	m_max_sequence(MAX_SEQUENCE),
	m_max_rtt(MAX_RTT),	//in milliseconds
	m_rtt(0),		
	m_last_send_time(0),
	m_last_data_bytes(0),
	m_running(false),
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
void AsyncConnection::init(const int32_t id, const real timeout)
{
	// set connection's id
	m_id = id;
	// set connection's timeout
	m_timeout = timeout;
}

//-----------------------------------------------------------------------------
bool AsyncConnection::start(uint16_t port)
{
	// if connection is not running
	assert(!m_running);
	os::printf("Start connection on port %d\n", port);
	// open socket
	if (!m_socket.open(port))
	{
		return false;
	}
	m_running = true;
	return true;  
}

//-----------------------------------------------------------------------------
void AsyncConnection::stop()
{
	// if connection is running
	assert(m_running);
	os::printf("stop connection\n");;
//  	bool connected = is_connected();
	_clear_data();
	// close socket
	m_socket.close();
	m_running = false;
}

//-----------------------------------------------------------------------------
void AsyncConnection::listen()
{
	os::printf("server listening for connection...\n");;
	// Set connection mode and state
	_clear_data();
	m_mode = SERVER;
	m_state = LISTENING;  
}

//-----------------------------------------------------------------------------
void AsyncConnection::connect(const os::NetAddress& addr)
{
  	_clear_data();

	os::printf("client connecting to ");
	os::printf("%i.", (uint8_t)addr.address[0]);
	os::printf("%i.", (uint8_t)addr.address[1]);
	os::printf("%i.", (uint8_t)addr.address[2]);
	os::printf("%i:", (uint8_t)addr.address[3]);
	os::printf("%i\n", (uint16_t)addr.port);

	m_mode = CLIENT;
	m_state = CONNECTING;
	m_remote_address = addr;
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
uint16_t AsyncConnection::get_local_sequence() const
{
	return m_local_sequence;
}
//-----------------------------------------------------------------------------
void AsyncConnection::send_message(BitMessage& msg, const uint32_t time)
{
	assert(m_running);
	
	m_socket.send(m_remote_address, msg.get_data(), msg.get_size());

}

//-----------------------------------------------------------------------------
int32_t AsyncConnection::receive_message(BitMessage& msg, const uint32_t time)
{
	assert(m_running);
	// init BitMessage handler
	msg.init(175);
	msg.begin_writing();
	size_t size = 175;
	// NetAddress handler
	os::NetAddress sender(0, 0, 0, 0, 0);
	// receive message
	int32_t bytes = m_socket.receive(sender, msg.get_data(), size);
	//TODO: why received bytes is zero
	os::printf("%d bytes received\n", bytes);
	msg.set_size(size);
	// sets BitMessage in only-read
	msg.begin_reading();
	
	if (m_mode == SERVER && !is_connected() && bytes > 0)
	{
		os::printf("server accepts connection from client ");
		os::printf("%i.", (uint8_t)sender.address[0]);
		os::printf("%i.", (uint8_t)sender.address[1]);
		os::printf("%i.", (uint8_t)sender.address[2]);
		os::printf("%i:", (uint8_t)sender.address[3]);
		os::printf("%i\n", (uint16_t)sender.port);
		
		m_state = CONNECTED;
		m_remote_address = sender;
	}

	if (sender == m_remote_address)
	{
		if (m_mode == CLIENT && m_state == CONNECTING)
		{
			os::printf("client completes connection with server");
			m_state = CONNECTED;
		}
		m_timeout_acc = 0.0f;
		return msg.get_size();
	}
	return 0;
}

//-----------------------------------------------------------------------------
void AsyncConnection::clear_reliable_messages()
{
	m_sent_msg.clear();
	m_received_msg.clear();
}

//-----------------------------------------------------------------------------
void AsyncConnection::update(real delta)
{
	assert(m_running);
	
	m_timeout_acc += delta;
	
	if (m_timeout_acc > m_timeout)
	{
		if (m_state == CONNECTING)
		{
			os::printf("Connect timed out\n");
			_clear_data();
			m_state = CONNECT_FAIL;
		}
		else if (m_state == CONNECTED)
		{
			os::printf("Connection timed out\n");
 			_clear_data();
			if (m_state == CONNECTING)
			{
				m_state = CONNECT_FAIL;
			}
		}
	}	
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
bool AsyncConnection::is_connecting() const
{ 
	return m_state == CONNECTING; 
}

//-----------------------------------------------------------------------------
bool AsyncConnection::is_listening() const
{ 
	return m_state == LISTENING; 
}

//-----------------------------------------------------------------------------
bool AsyncConnection::is_connected() const 
{ 
	return m_state == CONNECTED; 
}

//-----------------------------------------------------------------------------
bool AsyncConnection::is_connect_fail() const
{ 
	return m_state == CONNECT_FAIL; 
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

void AsyncConnection::_clear_data()
{
	m_state = DISCONNECTED;
	m_timeout_acc = 0.0f;
	m_remote_address = os::NetAddress();
}

} // namespace network
} // namespace crown
