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
	m_max_rtt(MAX_RTT),	//in milliseconds
	m_rtt(0),
	m_timeout(DEFAULT_TIMEOUT),
	m_timeout_acc(0),
	m_sent_packets(0),
	m_recv_packets(0),
	m_dropped_packets(0.0f),
	m_local_sequence(0),
	m_remote_sequence(0),
	m_max_sequence(MAX_SEQUENCE),
	m_sent_queue(allocator),
	m_received_queue(allocator),
	m_running(false),
	m_sent_packet(allocator),
	m_received_packet(allocator),
	m_pending_ack(allocator),
	m_acked(allocator),
	m_allocator(&allocator)
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
	os::printf("stopping connection...\n");;

	if (is_connected())
	{
		os::printf("connection stopped\n");;
		_clear_data();
		// close socket
		m_socket.close();
		m_running = false;
		return;
	}
	
	os::printf("connection is running yet");
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
	assert(m_running);
	
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
  
}

//-----------------------------------------------------------------------------
int AsyncConnection::get_incoming_rate() const
{
  
}

//-----------------------------------------------------------------------------
float AsyncConnection::get_packets_loss() const
{
	return m_dropped_packets;
}

//-----------------------------------------------------------------------------
uint16_t AsyncConnection::get_local_sequence() const
{
	return m_local_sequence;
}

//-----------------------------------------------------------------------------
uint16_t AsyncConnection::get_remote_sequence() const
{
	return m_remote_sequence;
}

//-----------------------------------------------------------------------------
void AsyncConnection::send_message(BitMessage& msg, const uint32_t time)
{
	assert(m_running);
	// set header
	msg.set_header(m_id, m_local_sequence, m_remote_sequence, _generate_ack_bits());
	// evaluate message size	
	size_t size = msg.get_header_size() + msg.get_size();
	uint8_t* data = (uint8_t*)m_allocator->allocate(size);
	// merge header with data 
	memcpy(data, msg.get_header(), 12);
	memcpy(data + 12, msg.get_data(), size - 12);
	// send message
	m_socket.send(m_remote_address, data, size);
	
	_packet_sent(msg.get_size());
	// storage outgoing message
	m_sent_queue.push_back(msg);
}

//-----------------------------------------------------------------------------
int32_t AsyncConnection::receive_message(BitMessage& msg, const uint32_t time)
{
	assert(m_running);
	
	os::NetAddress sender(0, 0, 0, 0, 0);

	size_t size = msg.get_max_size();
	
	uint8_t* data = (uint8_t*)m_allocator->allocate(msg.get_header_size() + size);

	// receive message
	int32_t received_bytes = m_socket.receive(sender, data, size);
	if (received_bytes <= 0)
	{
		return 0;
	}

	// header-taking
	uint32_t protocol_id = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
	uint16_t sequence = data[4] << 8 | data[5];
	uint16_t ack = data[6] << 8 | data[7];
	uint32_t ack_bits = data[8] << 24 | data[9] << 16 | data[10] << 8 | data[11];
	
	msg.begin_writing();
	msg.set_header(protocol_id, sequence, ack, ack_bits);
	
	//data-taking
 	uint8_t* tmp_ptr = &data[12];
	memcpy(msg.get_data(), tmp_ptr, msg.get_max_size());
	msg.set_size(size);
	// sets BitMessage in read-only for processing
	msg.begin_reading();
	
	_packet_received(sequence, msg.get_size());
	_process_ack(ack, ack_bits);
 	// storage incoming message	
	m_received_queue.push_back(msg);
	
	// establish connection after first packet is received
	if (m_mode == SERVER && !is_connected())
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
	
	// completes connection after first packet is received from server
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
	m_sent_queue.clear();
	m_received_queue.clear();
}

//-----------------------------------------------------------------------------
void AsyncConnection::update(real delta)
{
	assert(m_running);
	// improve timeout accumulator of delta time
	m_timeout_acc += delta;
	// if timeout accumulator > generic timeout
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
  
	PacketData tmp;
	
	tmp.sequence = m_local_sequence;

	// If local_sequence_number exists
	PacketData* h_ptr;
	h_ptr = std::find(m_sent_packet.begin(), m_sent_packet.end(), tmp);
	if (h_ptr != m_sent_packet.end())
	{
		seq_exists_in_sent_queue = true;
	}

	h_ptr = std::find(m_pending_ack.begin(), m_pending_ack.end(), tmp);
	if(h_ptr != m_pending_ack.end())
	{
		seq_exists_in_pending_ack_queue = true;
	}	
	// Else
	assert(!seq_exists_in_sent_queue);
 	assert(!seq_exists_in_pending_ack_queue);
	
	// Creates Header for saving in queues
	PacketData packet;
	packet.sequence = m_local_sequence;
	packet.time = 0.0f;
	packet.size = size;
	// Push packet data in sent_queue
	m_sent_packet.push_back(packet);
	// push packet data in pending_ack_queue
	m_pending_ack.push_back(packet);
	// Increments sent packets
	m_sent_packets++;
	// Increments local sequence
	m_local_sequence++;

	if (m_local_sequence >= MAX_SEQUENCE)
	{
		m_local_sequence = 0;
	}
}

//-----------------------------------------------------------------------------
void AsyncConnection::_packet_received(uint16_t sequence, size_t size)
{
	PacketData tmp;
	PacketData* h_ptr;

	tmp.sequence = sequence;

	// Increment received packets
	m_recv_packets++;
	
	// If packet's sequence exists, return
	h_ptr = std::find(m_received_packet.begin(), m_received_packet.end(), tmp);
	if (h_ptr != m_received_packet.end())
	{
		return;
	}
	
	PacketData packet;
	packet.sequence = sequence;
	packet.time = 0.0f;
	packet.size = size;
	// Push packet data in received_queue
	m_received_packet.push_back(packet);
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
	
	uint32_t index = 0;
	
	PacketData* i = m_pending_ack.begin();
	for(i = m_pending_ack.begin(), index = 0; i != m_pending_ack.end(); i++, index++)
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
  
		PacketData packet;
		if (acked)
		{
			m_rtt += (i->time - m_rtt) * 0.1f;
			m_acked.push_back(*i);
			m_pending_ack[index] = packet;
		}
	}
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
int32_t AsyncConnection::_generate_ack_bits()
{
	int32_t ack_bits = 0;
	
	for (PacketData* i = m_received_packet.begin(); i != m_received_packet.end(); i++)
	{
		if (i->sequence == m_remote_sequence || _sequence_more_recent(i->sequence, m_remote_sequence))
		{
			break;
		}
		
        int32_t bit_index = _bit_index_for_sequence(i->sequence, m_remote_sequence);
		if (bit_index <= 31)
		{
			ack_bits |= 1 << bit_index;
		}
	}
	return ack_bits;
}

//-----------------------------------------------------------------------------
void AsyncConnection::_update_packet_queues(uint32_t delta)
{
	PacketData* i;
	
	// updates queues time
	for (i = m_sent_packet.begin(); i != m_sent_packet.end(); i++)
	{
		i->time += delta;
	}
	for (i = m_received_packet.begin(); i != m_received_packet.end(); i++)
	{
		i->time += delta;
	}
	for (i = m_pending_ack.begin(); i != m_pending_ack.end(); i++)
	{
		i->time += delta;
	}
	for (i = m_acked.begin(); i != m_acked.end(); i++)
	{
		i->time += delta;
	}
	
	// cleans queues from old packets
	/*
	while (m_sent_packet.size() && m_sent_packet.front().time > MAX_RTT)
	{
		m_sent_packet.pop_front();
	}
	if (m_received_packet.size())
	{
		while()
	}
	while (m_acked.size() && m_acked.front().time > MAX_RTT * 2)
	{
		m_acked.pop_front();
	}
	while (m_pending_ack.size() && m_pending_ack.front().time > MAX_RTT)
	{
		m_pending_ack
		m_dropped_packets++;
	}*/

}

//-----------------------------------------------------------------------------
void AsyncConnection::_update_stats()
{
  
}


//-----------------------------------------------------------------------------
void AsyncConnection::_clear_data()
{
	m_state = DISCONNECTED;
	m_timeout_acc = 0.0f;
	m_remote_address = os::NetAddress();
}

} // namespace network
} // namespace crown
