#include "Connection.h"

namespace crown
{
namespace network
{

Connection::Connection()
{
}

Connection::~Connection()
{
}

void Connection::init(const NetAddress addr, const int id)
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
	m_unsent_fragments = false;
	m_unsent_fragment_start = 0;
	m_fragment_sequence = 0;
	m_fragment_length = 0;
	
	//TODO: init queues
}

void Connection::shutdown()
{
	//TODO: dealloc all
}

void Connection::reset_rate()
{
  	m_last_send_time = 0;
	m_last_data_bytes = 0;
	m_outgoing_rate_time = 0;
	m_outgoing_rate_bytes = 0;
	m_incoming_rate_time = 0;
	m_incoming_rate_bytes = 0;
}

void Connection::set_max_outgoing_rate(int rate)
{
	m_max_rate = rate;
}

int Connection::get_max_outgoing_rate()
{
	return m_max_rate;
}

os::NetAddress Connection::get_remote_address() const
{
	return m_remote_address;
}

int Connection::get_outgoing_rate() const
{
	return m_outgoing_rate_bytes;
}

int Connection::get_incoming_rate() const
{
	return m_incoming_rate_bytes;
}

float Connection::get_incoming_packet_loss() const
{
	
}

bool Connection::ready_to_send(const int time) const
{
  
}

int Connection::send_message(os::UDPSocket &socket, const int time, const BitMessage &msg)
{
  
}

void Connection::send_next_fragment(os::UDPSocket &socket, const int time)
{
  
}

bool Connection::unsent_fragments_left() const
{
  
}

bool Connection::process(const NetAddress from, BitMessage &msg, int &sequence, int time)
{
  
}

void Connection::send_reliable_message(const BitMessage &msg)
{
	m_reliable_send.push_back(msg);
}

bool Connection::get_reliable_message(BitMessage &msg)
{
	
}

void Connection::clear_reliable_messages()
{
	m_reliable_send = NULL;
	m_reliable_receive = NULL;
}

void Connection::_write_message(BitMessage &out, const BitMessage &msg)
{
  
}

bool Connection::_read_message(BitMessage &out, const BitMessage &msg)
{
 
}

void Connection::_update_outgoing_rate(const int time, const int size)
{
  
}

void Connection::_update_incoming_rate(const int time, const int size)
{
  
}

void Connection::_update_packet_loss(const int time, const int num_recv, const int num_dropped)
{
  
}

} // namespace network
} // namespace crown
