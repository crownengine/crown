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
	#define MAX_PACKET_LEN		1400
	#define MAX_MESSAGE_SIZE	16384
	#define MAX_QUEUE_SIZE		16384
	
 
class Connection
{
public:
  
							Connection(Allocator& allocator);
							~Connection();

	void					init(const os::NetAddress addr, const int32_t id = DEFAULT_PROTOCOL_ID);
	void					reset_rate();

							// Sets the maximum outgoing rate.
	void					set_max_outgoing_rate(int32_t rate);
							// Gets the maximum outgoing rate.
	int32_t					get_max_outgoing_rate();
							// Returns the address of the entity at the other side of the channel.
	os::NetAddress			get_remote_address() const;
							// Returns the average outgoing rate over the last second.
	int32_t					get_outgoing_rate() const;
							// Returns the average incoming rate over the last second.
	int32_t					get_incoming_rate() const;
							// Returns the average incoming packet loss over the last 5 seconds.
	real					get_incoming_packet_loss() const;		
							// Returns true if the channel is ready to send new data based on the maximum rate.
	bool					ready_to_send(const int32_t time) const;
							// Processes the incoming message.
	bool					process(const os::NetAddress from, int32_t time, BitMessage &msg, int32_t &sequence);
							// Sends a reliable message, in order and without duplicates.
	void					send_reliable_message(const BitMessage &msg);
							// Returns true if a new reliable message is available and stores the message.
	bool					receive_reliable_message(BitMessage &msg);
							// Removes any pending outgoing or incoming reliable messages.
	void					clear_reliable_messages();

private:
							// methods which provides a reliability system
	void					_update_outgoing_rate(const int32_t time, const int32_t size);
	void					_update_incoming_rate(const int32_t time, const int32_t size);
	void					_update_packet_loss(const int32_t time, const int32_t num_recv, const int32_t num_dropped);

private:
  
	os::NetAddress			m_remote_address;			// address of remote host
	int32_t					m_id;						// our identification used instead of port number
	int32_t					m_max_rate;					// maximum number of bytes that may go out per second

				// variables to control the outgoing rate
	int32_t					m_last_send_time;			// last time data was sent out
	int32_t					m_last_data_bytes;			// bytes left to send at last send time

				// variables to keep track of the rate
	int32_t					m_outgoing_rate_time;		// outgoing time rate
	int32_t					m_outgoing_rate_bytes;		// outgoing bytes rate
	int32_t					m_incoming_rate_time;		// incoming time rate
	int32_t					m_incoming_rate_bytes;		// incoming bytes rate

				// variables to keep track of the incoming packet loss
	real					m_incoming_recv_packets;
	real					m_incoming_dropped_packets;
	int32_t						m_incoming_packet_loss_time;

				// sequencing variables
	int32_t					m_outgoing_sequence;
	int32_t					m_incoming_sequence;

				// reliable messages
	Queue<BitMessage>		m_reliable_send;
	Queue<BitMessage>		m_reliable_receive;
};

} // namespace network
} // namespace crown
