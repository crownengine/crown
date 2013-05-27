/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include "Assert.h"
#include "Types.h"
#include "OS.h"
#include "TCPSocket.h"
#include "NetAddress.h"

namespace crown
{
namespace os
{

//-----------------------------------------------------------------------------
TCPSocket::TCPSocket() :
	m_socket(0),
	m_active_socket(0)

{
}

//-----------------------------------------------------------------------------
TCPSocket::~TCPSocket()
{
	close();
}

//-----------------------------------------------------------------------------
bool TCPSocket::open(uint16_t port)
{
	int sd = socket(AF_INET, SOCK_STREAM, 0);

	if (sd <= 0)
	{
		os::printf("failed to open socket\n");
		m_socket = 0;

		return false;
	}

	m_socket = sd;

	// Bind socket
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if (bind(sd, (const sockaddr*) &address, sizeof(sockaddr_in)) < 0)
	{
		os::printf("Failed to bind socket\n");
		close();

		return false;
	}

	listen(sd, 5);
	os::printf("Listening on port %d\n", port);

	sockaddr_in client;
	size_t client_length = sizeof(client);

	int asd = accept(sd, (sockaddr*)&client, (socklen_t*)&client_length);

	if (asd < 0)
	{
		os::printf("failed to accept connection\n");

		return false;
	}

	m_active_socket = asd;

	return true;
}

//-----------------------------------------------------------------------------
bool TCPSocket::connect(const NetAddress& destination)
{
	int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sd <= 0)
	{
		os::printf("Failed to open socket\n");
		m_socket = 0;

		return false;
	}

	m_socket = sd;

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr =  htonl(destination.address());
	address.sin_port = htons(destination.port());

	if (::connect(sd, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
	{
		os::printf("Failed to connect socket\n");
		close();

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
void TCPSocket::close()
{
	if (m_active_socket != 0)
	{
		::close(m_active_socket);
		m_active_socket = 0;  
	}

	if (m_socket != 0)
	{
		::close(m_socket);
		m_socket = 0;
	}
}

//-----------------------------------------------------------------------------
bool TCPSocket::send(const void* data, size_t size)
{
	CE_ASSERT(data != NULL, "Data must be != NULL");

	if (m_active_socket <= 0)
	{
		m_socket = 0;
		m_active_socket = 0;

		return false;
	}

	ssize_t sent_bytes = ::send(m_active_socket, (const char*) data, size, 0);
	if (sent_bytes <= 0)
	{
		os::printf("Unable to send data");
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
size_t TCPSocket::receive(void* data, size_t size)
{
	CE_ASSERT(data != NULL, "Data must be != NULL");

	if (m_active_socket <= 0)
	{
		return false;
	}

	ssize_t received_bytes = ::recv(m_active_socket, (char*) data, size, 0);
	if (received_bytes <= 0)
	{
		return 0;
	}

	return received_bytes;
}

//-----------------------------------------------------------------------------
bool TCPSocket::is_open()
{
	return m_active_socket != 0 || m_socket != 0;
}

//-----------------------------------------------------------------------------
int	TCPSocket::socket_id()
{
	return m_socket;
}

//-----------------------------------------------------------------------------
int	TCPSocket::active_socket_id()
{
	return m_active_socket;
}
		
} // namespace os
} // namespace crown
