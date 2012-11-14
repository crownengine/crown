#include <cassert>
#include <winsock2.h>
#pragma comment(lib, "wsock32.lib")

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

bool UDPSocket::open(ushort port)
{
	assert(!is_open());

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket <= 0)
	{
		os::printf("Failed to create socket.");
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
		os::printf("Failed to bind socket");
		close();
		return false;
	}

	// set non-blocking io
	DWORD non_blocking = 1;
	if (ioctlsocket( socket, FIONBIO, &non_blocking ) != 0)
	{
		os::printf("Failed to set non-blocking socket");
		close();
		return false;
	}

	return true;  
}

bool UDPSocket::send(IPv4Address &receiver, const void* data, int size )
{
	assert(data);
	assert(size > 0);

	if (m_socket == 0)
	{
		return false;
	}
	
	assert(receiver.get_address() != 0);
	assert(receiver.get_port() != 0);

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(receiver.get_address());
	address.sin_port = htons(receiver.get_port());

	int sent_bytes = sendto(m_socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));

	return sent_bytes == size;  
}

int UDPSocket::receive(IPv4Address &sender, void* data, int size)
{
	assert(data);
	assert(size > 0);

	if (m_socket == 0)
	{
		return false;
	}

	typedef int socklen_t;

	sockaddr_in from;
	socklen_t from_length = sizeof(from);

	int received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &from_length);

	if (received_bytes <= 0)
	{
			return 0;
	}

	uint address = ntohl(from.sin_addr.s_addr);
	ushort port = ntohs(from.sin_port);

	sender.set(address, port);

	return received_bytes;
}

void UDPSocket::close()
{
	if ( m_socket != 0 )
	{
		closesocket(m_socket);
		m_socket = 0;
	}  
}

bool UDPSocket::is_open()
{
	return m_socket != 0; 
}

}
}

