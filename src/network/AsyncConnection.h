#pragma once

#include "Types.h"
#include "OS.h"
#include "Allocator.h"
#include "BitMessage.h"
#include "Queue.h"

namespace crown
{
namespace network
{

/**
 * Reliable connection over UDP
 */
class AsyncConnection
{
public:
  
	struct PacketData
	{
		  uint32_t 	sequence;
		  real		time;
		  size_t	size;
		  
		  PacketData()
		  {
			  sequence = 0;
			  time = 0.0f;
			  size = 0;
		  }
		  
		  bool operator==(const PacketData& packet)
		  {
			  return sequence == packet.sequence;
		  }
	};
  
public:

									AsyncConnection(Allocator& allocator);
									~AsyncConnection();

	void							init(const int32_t id = DEFAULT_PROTOCOL_ID, const real timeout = DEFAULT_TIMEOUT);
	bool 							start(uint16_t port);
	void 							stop();
	void 							listen();
	void 							connect(const os::NetAddress& addr);
	void							reset_rate();

								  // Sets the maximum outgoing rate.
	void							set_max_outgoing_rate(int32_t rate);
									// Gets the maximum outgoing rate.
	int32_t							get_max_outgoing_rate();
									// Returns the address of the entity at the other side of the channel.
	os::NetAddress					get_remote_address() const;
									// Returns the average outgoing rate over the last second.
	int32_t							get_outgoing_rate() const;
									// Returns the average incoming rate over the last second.
	int32_t							get_incoming_rate() const;
									// Returns the average incoming packet loss over the last 5 seconds.
	real							get_packets_loss() const;
									// returns the current local sequence number
	uint16_t						get_local_sequence() const;
									// returns the current remote sequence number
	uint16_t						get_remote_sequence() const;
									// Sends message
	void							send_message(BitMessage& msg, const uint32_t time);
									// Receive message and process
	int32_t							receive_message(BitMessage& msg, const uint32_t time);
									// Removes any pending outgoing or incoming reliable messages.
	void							clear_reliable_messages();
									// Update AsyncConnection
	void 							update(real delta);
									// Returns true if the connection is ready to send new data based on the maximum rate.
	bool							ready_to_send(const int32_t time) const;
	
	bool 							is_connecting() const;
	bool 							is_listening() const; 
	bool 							is_connected() const;
	bool 							is_connect_fail() const;
	
private:
	
	// connection mode
	enum Mode
	{
		NONE,
		CLIENT,
		SERVER
	};
	
	// connection states
	enum State
	{
		DISCONNECTED,
		LISTENING,
		CONNECTING,
		CONNECT_FAIL,
		CONNECTED
	};  
  
private:

									 // methods which provides a reliability system
	void 							_packet_sent(size_t size);
	void 							_packet_received(uint16_t sequence, size_t size);
	void 							_process_ack(uint16_t ack, int32_t ack_bits);
	bool							_sequence_more_recent(uint16_t s1, uint16_t s2);
	int32_t							_bit_index_for_sequence(uint16_t seq, uint16_t ack);
	int32_t							_generate_ack_bits();
	void							_update_stats();
	void							_update_packet_queues(uint32_t delta);
// 	void							_insert_sorted_in_queue(PacketData* begin, PacketData* end);
									// methods which provides a flow control system

	
	void							_clear_data();


private:
  
	os::NetAddress					m_remote_address;			// address of remote host
	os::UDPSocket					m_socket;					// socket
	uint32_t						m_id;						// our identification used instead of port number
	Mode							m_mode;						// connection mode
	State							m_state;					// connection state
	uint32_t						m_max_rate;					// maximum number of bytes that may go out per second
	real							m_max_rtt;					// Maximum round trip time
	real							m_rtt;						// Round trip time
	real							m_timeout;					// connection timeout value
	real							m_timeout_acc;				// timeout accumulator

									// variables to keep track of the incoming packet loss
	uint32_t						m_sent_packets;		
	uint32_t						m_recv_packets;
	real							m_dropped_packets;

									// sequencing variables
	uint16_t						m_local_sequence;
	uint16_t						m_remote_sequence;
	uint16_t						m_max_sequence;

									// message queues
	Queue<BitMessage>				m_sent_queue;
	Queue<BitMessage>				m_received_queue;
	
									// flag variables
	bool							m_running;
	
									// packet queues
	Queue<PacketData>				m_sent_packet;					// Sent messages queue
	Queue<PacketData>				m_received_packet;				// Received messages queue
	Queue<PacketData>				m_pending_ack;					// Pending acknokledges queue
	Queue<PacketData>				m_acked;						// Acknowledges queue
	
									// statistic variables
	real							m_sent_bytes_per_sec;
	real							m_acked_bytes_per_sec;
	real							m_sent_bandwidth;
	real							m_acked_bandwidth;
	
	Allocator*						m_allocator;					// dynamic allocator
									
									// constants
	static const uint32_t			DEFAULT_PROTOCOL_ID	= 0xFFFFFFFF;
	static const uint32_t			DEFAULT_TIMEOUT		= 10;
	static const uint32_t			MAX_RATE			= 64000;
	static const uint32_t			MAX_SEQUENCE		= 0xFFFF;
	static const uint32_t			MAX_RTT				= 1000;
	static const uint32_t			MAX_PACKET_LEN		= 1400;
	static const uint32_t			MAX_MESSAGE_SIZE	= 16384;
	static const uint32_t			MAX_QUEUE_SIZE		= 16384;

};

} // namespace network
} // namespace crown
