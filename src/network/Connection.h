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
	#define MAX_MESSAGE_SIZE	16384
	#define MAX_QUEUE_SIZE		16384
 
	class Connection
	{
	public:
	  
								Connection(Allocator& allocator);
								~Connection();
  
		void					init(const os::NetAddress addr, const int id);
		void					shutdown();
		void					reset_rate();

								// Sets the maximum outgoing rate.
		void					set_max_outgoing_rate(int rate);
								// Gets the maximum outgoing rate.
		int						get_max_outgoing_rate();
								// Returns the address of the entity at the other side of the channel.
		os::NetAddress			get_remote_address() const;
								// Returns the average outgoing rate over the last second.
		int						get_outgoing_rate() const;
								// Returns the average incoming rate over the last second.
		int						get_incoming_rate() const;
								// Returns the average incoming packet loss over the last 5 seconds.
		float					get_incoming_packet_loss() const;		
								// Returns true if the channel is ready to send new data based on the maximum rate.
		bool					ready_to_send(const int time) const;
								// Sends an unreliable message, in order and without duplicates.
		int						send_message(os::UDPSocket &socket, const int time, const BitMessage &msg);
		
								// Sends the next fragment if the last message was too large to send at once.
		void					send_next_fragment(os::UDPSocket &socket, const int time);
								// Returns true if there are unsent fragments left.
		bool					unsent_fragments_left() const;

								// Processes the incoming message. Returns true when a complete message
								// is ready for further processing. In that case the read pointer of msg
								// points to the first byte ready for reading, and sequence is set to
								// the sequence number of the message.
		bool					process(const os::NetAddress from, BitMessage &msg, int &sequence, int time);

								// Sends a reliable message, in order and without duplicates.
		void					send_reliable_message(const BitMessage &msg);
								// Returns true if a new reliable message is available and stores the message.
		bool					get_reliable_message(BitMessage &msg);
								// Removes any pending outgoing or incoming reliable messages.
		void					clear_reliable_messages();

	private:
								// methods which provides a pattern for communication
		void					_write_message(BitMessage &out, const BitMessage &msg);
		bool					_read_message(BitMessage &out, const BitMessage &msg);
  
								// methods which provides a reliability system
		void					_update_outgoing_rate(const int time, const int size);
		void					_update_incoming_rate(const int time, const int size);
		void					_update_packet_loss(const int time, const int num_recv, const int num_dropped);

	private:
	  
		os::NetAddress				m_remote_address;		// address of remote host
		int						m_id;					// our identification used instead of port number
		int						m_max_rate;			// maximum number of bytes that may go out per second

					// variables to control the outgoing rate
		int						m_last_send_time;		// last time data was sent out
		int						m_last_data_bytes;	// bytes left to send at last send time

					// variables to keep track of the rate
		int						m_outgoing_rate_time;		// outgoing time rate
		int						m_outgoing_rate_bytes;		// outgoing bytes rate
		int						m_incoming_rate_time;		// incoming time rate
		int						m_incoming_rate_bytes;		// incoming bytes rate

					// variables to keep track of the incoming packet loss
		float					m_incoming_recv_packets;
		float					m_incoming_dropped_packets;
		int						m_incoming_packet_loss_time;
  
					// sequencing variables
		int						m_outgoing_sequence;
		int						m_incoming_sequence;

					// outgoing fragment buffer
		bool					m_unsent_fragments;
		int						m_unsent_fragment_start;
		uint8_t					m_unsent_buffer[MAX_MESSAGE_SIZE];
		BitMessage				m_unsent_msg;

					// incoming fragment assembly buffer
		int						m_fragment_sequence;
		int						m_fragment_length;
		uint8_t					m_fragment_buffer[MAX_MESSAGE_SIZE];

					// reliable messages
		Queue<BitMessage>		m_reliable_send;
		Queue<BitMessage>		m_reliable_receive;
	  
	};

} // namespace network
} // namespace crown
