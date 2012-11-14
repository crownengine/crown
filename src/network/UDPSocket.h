#pragma once

namespace crown
{
	class IPv4Address;
	
	class UDPSocket
	{
	public:

					// Constructor
					UDPSocket();
					// Destructor
					~UDPSocket();
					// Open connection
		bool 		open(unsigned short port);
					 // Send data through socket
		bool 		send(IPv4Address &receiver, const void* data, int size );
					// Receive data through socket
		int 		receive(IPv4Address &sender, void* data, int size);
					// Close connection
		void 		close();
					// Is connection open?
		bool 		is_open();

	private:
					// Socket descriptor
		int 		m_socket;
	};	
}
