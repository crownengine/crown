#include <cassert>
#include <winsock2.h>
#pragma comment(lib, "wsock32.lib")


#include "OS.h"

namespace crown
{
namespace os
{
 
TCPSocket::TCPSocket()
{
	set_socket_id(0);
	set_active_socket_id(0);  
}

TCPSocket::~TCPSocket()
{
	close();
}

bool TCPSocket::open(unsigned short port)
{
	int sd = socket(AF_INET, SOCK_STREAM, 0);

	if (sd <= 0)
	{
		os::printf("failed to open socket\n");
		set_socket_id(0);
		return false;
	}

	set_socket_id(sd);

	// Bind socket
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if (bind(sd, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
	{
		os::printf("failed to bind socket\n");
		close();
		return false;
	}

	listen(sd, 5);
	os::printf("listening on port %d", port);

	sockaddr_in client;
	uint client_length = sizeof(client);

	int active_sd = accept(sd, (sockaddr*)&client, &client_length);
	if (active_sd < 0)
	{
		os::printf("failed to accept connections");
	}

	set_active_socket_id(active_sd);

	return true;  
}

bool TCPSocket::connect(IPv4Address& destination)
{		
	int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sd <= 0)
	{
		os::printf("failed to open socket\n");
		set_socket_id(0);
		return false;
	}
	
	set_socket_id(sd);

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr =  htonl(destination.get_address());
	address.sin_port = htons(destination.get_port());

	if (::connect(sd, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
	{
		os::printf("failed to connect socket\n");
		close();
		return false;
	}
}

int	TCPSocket::close()
{
		int asd = get_active_socket_id();
		if (asd != 0)
		{
			closesocket(asd);
			set_active_socket_id(0);
		}

		int sd = get_socket_id();
		if (sd != 0)
		{
			closesocket(sd);
			set_socket_id(0);
		}
}

bool TCPSocket::send(const void* data, int size)
{
	assert(data);
	assert(size > 0);

	int sd = get_active_socket_id();
	if (sd <= 0)
	{
		set_socket_id(0);
		set_active_socket_id(0);
		return false;
	}

	int sent_bytes = ::send(sd, (const char*)data, size, 0);
	if (sent_bytes <= 0)
	{
		os::printf("Unable to send data");
		return false;
	}

	return true;  
}

int	TCPSocket::receive(void* data, int size)
{
	assert(data);
	assert(size > 0);

	int sd = get_active_socket_id();

	if ( sd <= 0 )
	{
		return false;
	}

	int received_bytes = recv(sd, (char*)data, size, 0);
	if ( received_bytes <= 0 )
	{
		return 0;
	}

	return received_bytes;  
}

bool TCPSocket::is_open()
{
	return m_active_socket != 0 || m_socket != 0;
}

int TCPSocket::get_socket_id()
{
	return m_socket; 
}

int	TCPSocket::get_active_socket_id()
{
	return m_active_socket != 0 ? m_active_socket : m_socket; 
}

void TCPSocket::set_socket_id(int socket)
{
	m_socket = socket;
}

void TCPSocket::set_active_socket_id(int socket)
{
	m_active_socket = socket;
}
	
}
}

