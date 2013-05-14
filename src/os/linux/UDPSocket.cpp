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
#include <cassert>

#include "Types.h"
#include "OS.h"
#include "UDPSocket.h"
#include "NetAddress.h"

namespace crown
{ 
namespace os
{

//-----------------------------------------------------------------------------
UDPSocket::UDPSocket() :
	m_socket(0)
{
}

//-----------------------------------------------------------------------------
UDPSocket::~UDPSocket()
{
	close();
}

//-----------------------------------------------------------------------------
bool UDPSocket::open(uint16_t port)
{
	assert(!is_open());

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socket <= 0)
	{
		os::printf("Failed to create socket.\n");
		m_socket = 0;

		return false;
	}

	// Bind to port
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(m_socket, (const sockaddr*) &address, sizeof(sockaddr_in)) < 0)
	{
		os::printf("Failed to bind socket\n");
		close();

		return false;
	}

	int non_blocking = 1;

	if (fcntl(m_socket, F_SETFL, O_NONBLOCK, non_blocking) == -1)
	{
		os::printf("Failed to set non-blocking socket\n");
		close();

		return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
bool UDPSocket::send(const NetAddress &receiver, const void* data, size_t size)
{
	assert(is_open());
	assert(data != NULL);
	assert(size > 0);

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(receiver.address());
	address.sin_port = htons(receiver.port());

	ssize_t sent_bytes = sendto(m_socket, (const char*) data, size, 0, (sockaddr*) &address, sizeof(sockaddr_in));

	if (sent_bytes < 0)
	{
		return false;
	}

	return (size_t) sent_bytes == size;
}

//-----------------------------------------------------------------------------
size_t UDPSocket::receive(NetAddress& sender, void* data, size_t size)
{
	assert(is_open());
	assert(data);
	assert(size > 0);

	sockaddr_in from;
	socklen_t from_length = sizeof(from);

	ssize_t received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &from_length);

	if (received_bytes <= 0)
	{
		return 0;
	}

	uint32_t address = ntohl(from.sin_addr.s_addr);
	uint16_t port = ntohs(from.sin_port);

	sender.set(address, port);

	return (size_t) received_bytes;
}

//-----------------------------------------------------------------------------
void UDPSocket::close()
{
	if (m_socket != 0)
	{
		::close(m_socket);
		m_socket = 0;
	}
}

//-----------------------------------------------------------------------------
bool UDPSocket::is_open()
{
	return m_socket != 0;
}

} // namespace os
} // namespace crown
