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
	#define DEFAULT_PROTOCOL_ID 0xFFFFFFFF
	
	#define MAX_SEQUENCE		0xFFFFFFFF
	#define MAX_PACKET_LEN		1400
	#define MAX_MESSAGE_SIZE	16384
	#define MAX_QUEUE_SIZE		16384
	
/**
 * Reliable connection over UDP
 */
class AsyncConnection
{
public:
  
									AsyncConnection(Allocator& allocator);
									~AsyncConnection();

	void							init(const os::NetAddress addr, const int32_t id = DEFAULT_PROTOCOL_ID);
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
	real							get_incoming_packet_loss() const;		
									// Sends message
	void							send_message(BitMessage& msg, const uint32_t time);
									// Receive message
	bool							receive_message(BitMessage& msg, const uint32_t time);
									// Removes any pending outgoing or incoming reliable messages.
	void							clear_reliable_messages();
									// Update AsyncConnection
	void 							update(real delta);
									// Returns true if the connection is ready to send new data based on the maximum rate.
	bool							ready_to_send(const int32_t time) const;
									// Processes the incoming message.
	bool							process(const os::NetAddress from, int32_t time, BitMessage &msg, int32_t &sequence);

private:

									 // methods which provides a reliability system
	void 							_packet_sent(size_t size);
	void 							_packet_received(uint16_t sequence, size_t size);
	void 							_process_ack(uint16_t ack, int32_t ack_bits);
	bool							_sequence_more_recent(uint16_t s1, uint16_t s2);
	int32_t							_bit_index_for_sequence(uint16_t seq, uint16_t ack);
	int32_t							_generate_ack_bits(uint16_t ack);
	void							_update_outgoing_rate(const uint32_t time, const size_t size);
	void							_update_incoming_rate(const uint32_t time, const size_t size);
	void							_update_packet_loss(const uint32_t time, const uint32_t num_recv, const uint32_t num_dropped);


private:
  
	os::NetAddress					m_remote_address;			// address of remote host
	os::UDPSocket					m_socket;					// socket
	int32_t							m_id;						// our identification used instead of port number
	int32_t							m_max_rate;					// maximum number of bytes that may go out per second

									// variables to keep track of the rate
	int32_t							m_outgoing_rate_time;		// outgoing time rate
	int32_t							m_outgoing_rate_bytes;		// outgoing bytes rate
	int32_t							m_incoming_rate_time;		// incoming time rate
	int32_t							m_incoming_rate_bytes;		// incoming bytes rate

									// variables to keep track of the incoming packet loss
	real							m_incoming_recv_packets;
	real							m_incoming_dropped_packets;
	int32_t							m_incoming_packet_loss_time;
	
						
	int32_t							m_outgoing_sent_packet;		// keep track of sent packet

									// sequencing variables
	int32_t							m_local_sequence;
	int32_t							m_remote_sequence;
	int32_t							m_max_sequence;
	
	real							m_max_rtt;					// Maximum round trip time
	real							m_rtt;						// Round trip time
	
									// variables to control the outgoing rate
	int32_t							m_last_send_time;			// last time data was sent out
	int32_t							m_last_data_bytes;			// bytes left to send at last send time

									// reliable messages
	Queue<BitMessage::Header>		m_sent_msg;					// Sent messages queue
	Queue<BitMessage::Header>		m_received_msg;				// Received messages queue
	Queue<BitMessage::Header>		m_pending_ack;				// Pending acknokledges queue
	Queue<BitMessage::Header>		m_acked;					// Acknowledges queue

};

} // namespace network
} // namespace crown
