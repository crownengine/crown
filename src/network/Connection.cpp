#include "Connection.h"

namespace crown
{
namespace network
{

Connection::Connection(Allocator& allocator) :
 	m_reliable_send(allocator),
 	m_reliable_receive(allocator)
{
  
}

//-----------------------------------------------------------------------------
Connection::~Connection()
{
  
}

//-----------------------------------------------------------------------------
void Connection::init(const os::NetAddress addr, const int id)
{
	m_remote_address = addr;
	m_id = id;
	m_max_rate = 64000;
	m_last_send_time = 0;
	m_last_data_bytes = 0;
	m_outgoing_rate_time = 0;
	m_outgoing_rate_bytes = 0;
	m_incoming_rate_time = 0;
	m_incoming_rate_bytes = 0;
	m_incoming_recv_packets = 0.0f;
	m_incoming_dropped_packets = 0.0f;
	m_incoming_packet_loss_time = 0;
	m_outgoing_sequence = 0;
	m_incoming_sequence = 0;
}

//-----------------------------------------------------------------------------
void Connection::shutdown()
{
	//TODO: don't needed with Allocator mechanism
}

//-----------------------------------------------------------------------------
void Connection::reset_rate()
{
  	m_last_send_time = 0;
	m_last_data_bytes = 0;
	m_outgoing_rate_time = 0;
	m_outgoing_rate_bytes = 0;
	m_incoming_rate_time = 0;
	m_incoming_rate_bytes = 0;
}

//-----------------------------------------------------------------------------
void Connection::set_max_outgoing_rate(int rate)
{
	m_max_rate = rate;
}

//-----------------------------------------------------------------------------
int Connection::get_max_outgoing_rate()
{
	return m_max_rate;
}

//-----------------------------------------------------------------------------
os::NetAddress Connection::get_remote_address() const
{
	return m_remote_address;
}

//-----------------------------------------------------------------------------
int Connection::get_outgoing_rate() const
{
	return m_outgoing_rate_bytes;
}

//-----------------------------------------------------------------------------
int Connection::get_incoming_rate() const
{
	return m_incoming_rate_bytes;
}

//-----------------------------------------------------------------------------
float Connection::get_incoming_packet_loss() const
{
	if (m_incoming_recv_packets == 0 && m_incoming_dropped_packets == 0)
	{
		return 0.0f;
	}
	// return loss packet %
	return m_incoming_dropped_packets * 100 / (m_incoming_recv_packets + m_incoming_dropped_packets);
}

//-----------------------------------------------------------------------------
bool Connection::ready_to_send(const int time) const
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
/* 
Processes the incoming message. Returns true when a complete message
is ready for further processing. In that case the read pointer of msg
points to the first byte ready for reading, and sequence is set to
the sequence number of the message.
*/
bool Connection::process(const os::NetAddress from, int time, BitMessage &msg, int &sequence)
{

}

//-----------------------------------------------------------------------------
void Connection::send_reliable_message(const BitMessage& msg)
{
	m_reliable_send.push_back(msg);
}

//-----------------------------------------------------------------------------
bool Connection::receive_reliable_message(BitMessage& msg)
{
}

//-----------------------------------------------------------------------------
void Connection::clear_reliable_messages()
{
	m_reliable_send.clear();
	m_reliable_receive.clear();
}

//-----------------------------------------------------------------------------
void Connection::_write_message(BitMessage& out, const BitMessage& msg)
{
	 uint8_t* packet; 
	 
}

//-----------------------------------------------------------------------------
bool Connection::_read_message(BitMessage& out, const BitMessage& msg)
{
 
}

//-----------------------------------------------------------------------------
void Connection::_update_outgoing_rate(const int time, const int size)
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
void Connection::_update_incoming_rate(const int time, const int size)
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
void Connection::_update_packet_loss(const int time, const int num_recv, const int num_dropped)
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
