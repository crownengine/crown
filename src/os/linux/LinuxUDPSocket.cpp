#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>

#include "Types.h"
#include "OS.h"

namespace crown
{ 
namespace os
{

UDPSocket::UDPSocket()
{
	m_socket = 0;  
}
UDPSocket::~UDPSocket()
{
	close();
}

bool UDPSocket::open(uint16_t port)
{
  		assert(!is_open());

		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_socket <= 0)
		{
			os::printf("Failed to create socket.\n");;
			m_socket = 0;
			return false;
		}

		// bind to port
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		if ( bind( m_socket, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
		{
			os::printf("Failed to bind socket\n");
			close();
			return false;
		}

		int32_t non_blocking = 1;
		if (fcntl( m_socket, F_SETFL, O_NONBLOCK, non_blocking ) == -1)
		{
			os::printf("Failed to set non-blocking socket\n");
			close();
			return false;
		}
		
		return true;
}

bool UDPSocket::send(NetAddress &receiver, const void* data, size_t size)
{
	assert(data);
	assert(size > 0);

	if (m_socket == 0)
	{
		return false;
	}
	
	assert(receiver.address);
	assert(receiver.port);

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(receiver.get_address());
	address.sin_port = htons(receiver.get_port());

	int32_t sent_bytes = sendto(m_socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));

	return sent_bytes == size;
}

int32_t UDPSocket::receive(NetAddress& sender, void* data, size_t size)
{
	assert(data);
	assert(size > 0);

	if (m_socket == 0)
	{
		return false;
	}

	sockaddr_in from;
	socklen_t from_length = sizeof(from);

	int32_t received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &from_length);

	if (received_bytes <= 0)
	{
			return 0;
	}

	uint32_t address = ntohl(from.sin_addr.s_addr);
	uint16_t port = ntohs(from.sin_port);
	
	sender.set(address, port);
	
	return received_bytes;	
}

void UDPSocket::close()
{
	if ( m_socket != 0 )
	{
		::close(m_socket);
		m_socket = 0; 
	}
}

bool UDPSocket::is_open()
{
	return m_socket != 0;
}

} // namespace os
} // namespace crown
