/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "Types.h"
#include "NetAddress.h"
#include "Assert.h"
#include "OS.h"

namespace crown
{

struct ReadResult
{
	enum { NO_ERROR, UNKNOWN, REMOTE_CLOSED } error;
	size_t received_bytes;
};

struct WriteResult
{
	enum { NO_ERROR, UNKNOWN, REMOTE_CLOSED } error;
	size_t sent_bytes;
};

class TCPSocket
{
public:

	//-----------------------------------------------------------------------------
	TCPSocket()
		: m_socket(0)
	{
	}

	//-----------------------------------------------------------------------------
	TCPSocket(int socket)
		: m_socket(socket)
	{
	}

	//-----------------------------------------------------------------------------
	bool open(const NetAddress& destination, uint16_t port)
	{
		int sock_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (sock_id <= 0)
		{
			os::printf("Failed to open socket\n");
			m_socket = 0;

			return false;
		}

		m_socket = sock_id;

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr =  htonl(destination.address());
		address.sin_port = htons(port);

		if (::connect(sock_id, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
		{
			os::printf("Failed to connect socket\n");
			close();

			return false;
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	void close()
	{
		if (m_socket != 0)
		{
			::close(m_socket);
			m_socket = 0;
		}
	}

	//-----------------------------------------------------------------------------
	ReadResult read_nonblock(void* data, size_t size)
	{
		fcntl(m_socket, F_SETFL, O_NONBLOCK);
		ssize_t read_bytes = ::read(m_socket, (char*) data, size);

		ReadResult result;
		if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			result.error = ReadResult::NO_ERROR;
			result.received_bytes = 0;
		}
		else if (read_bytes == 0)
		{
			result.error = ReadResult::REMOTE_CLOSED;
		}
		else
		{
			result.error = ReadResult::NO_ERROR;
			result.received_bytes = read_bytes;
		}

		return result;		
	}

	//-----------------------------------------------------------------------------
	ReadResult read(void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		int flags = fcntl(m_socket, F_GETFL, 0);
		fcntl(m_socket, F_SETFL, flags & ~O_NONBLOCK);

		// Ensure all data is read
		char* buf = (char*) data;
		size_t to_read = size;
		ReadResult result;
		result.received_bytes = 0;
		result.error = ReadResult::NO_ERROR;

		while (to_read > 0)
		{
			ssize_t read_bytes = ::read(m_socket, buf, to_read);
			
			if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
			else if (read_bytes == 0)
			{
				result.error = ReadResult::REMOTE_CLOSED;
				return result;
			}

			buf += read_bytes;
			to_read -= read_bytes;
			result.received_bytes += read_bytes;
		}

		result.error = ReadResult::NO_ERROR;
		return result;
	}

	//-----------------------------------------------------------------------------
	WriteResult write_nonblock(const void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);
	}

	//-----------------------------------------------------------------------------
	WriteResult write(const void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		const char* buf = (const char*) data;
		size_t to_send = size;
		WriteResult result;
		result.sent_bytes = 0;
		result.error = WriteResult::NO_ERROR;

		// Ensure all data is sent
		while (to_send > 0)
		{
			ssize_t sent_bytes = ::send(m_socket, (const char*) buf, to_send, 0);

			// Check for errors
			if (sent_bytes == -1)
			{
				switch (errno)
				{
					case EAGAIN:
					{
						continue;
					}
					default:
					{
						result.error = WriteResult::UNKNOWN;
						return result;
					}
				}
			}

			buf += sent_bytes;
			to_send -= sent_bytes;
			result.sent_bytes += sent_bytes;
		}

		result.error = WriteResult::NO_ERROR;
		return result;
	}

public:

	int m_socket;
};

class TCPListener
{
public:

	//-----------------------------------------------------------------------------
	bool open(uint16_t port)
	{
		int& sock_id = m_listener.m_socket;

		sock_id = socket(AF_INET, SOCK_STREAM, 0);
		CE_ASSERT(sock_id != -1, "Failed to open socket: errno: %d", errno);

		fcntl(sock_id, F_SETFL, O_NONBLOCK);

		// Bind socket
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);

		int bind_ret = bind(sock_id, (const sockaddr*) &address, sizeof(sockaddr_in));
		CE_ASSERT(bind_ret != -1, "Failed to bind socket: errno: %d", errno);

		int listen_ret = ::listen(sock_id, 5);
		CE_ASSERT(listen_ret != -1, "Failed to listen on socket: errno: %d", errno);

		return true;
	}

	//-----------------------------------------------------------------------------
	void close()
	{
		m_listener.close();
	}

	//-----------------------------------------------------------------------------
	bool listen(TCPSocket& c)
	{
		int& sock_id = m_listener.m_socket;

		sockaddr_in client;
		size_t client_length = sizeof(client);

		int asd = accept(sock_id, (sockaddr*)&client, (socklen_t*)&client_length);

		if (asd == -1 && errno == EWOULDBLOCK)
		{
			return false;
		}

		c.m_socket = asd;

		return true;
	}

	//-----------------------------------------------------------------------------
	ReadResult read(void* data, size_t size)
	{
		return m_listener.read(data, size);
	}

	//-----------------------------------------------------------------------------
	WriteResult write(const void* data, size_t size)
	{
		return m_listener.write(data, size);
	}

private:

	TCPSocket m_listener;
};

class UDPSocket
{
public:

	//-----------------------------------------------------------------------------
	UDPSocket()
		: m_socket(0)
	{
	}

	//-----------------------------------------------------------------------------
	bool open(uint16_t port)
	{
		CE_ASSERT(!is_open(), "Socket is already open");

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

		if (fcntl(m_socket, F_SETFL, O_NONBLOCK, 1) == -1)
		{
			os::printf("Failed to set non-blocking socket\n");
			close();

			return false;
		}
		
		return true;
	}

	//-----------------------------------------------------------------------------
	void close()
	{
		if (m_socket != 0)
		{
			::close(m_socket);
			m_socket = 0;
		}
	}

	//-----------------------------------------------------------------------------
	ReadResult read(NetAddress& sender, uint16_t& port, const void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		sockaddr_in from;
		socklen_t from_length = sizeof(from);

		ssize_t received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &from_length);

		ReadResult result;

		if (received_bytes == -1 && errno == EAGAIN)
		{
			result.error = ReadResult::NO_ERROR;
			result.received_bytes = 0;
		}
		else if (received_bytes == 0)
		{
			result.error = ReadResult::REMOTE_CLOSED;
		}
		else
		{
			result.error = ReadResult::NO_ERROR;
			result.received_bytes = received_bytes;
		}

		sender.set(ntohl(from.sin_addr.s_addr));
		port = ntohs(from.sin_port);

		return result;
	}

	//-----------------------------------------------------------------------------
	WriteResult write(const NetAddress& receiver, uint16_t port, void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(receiver.address());
		address.sin_port = htons(port);

		ssize_t sent_bytes = sendto(m_socket, (const char*) data, size, 0, (sockaddr*) &address, sizeof(sockaddr_in));

		WriteResult result;

		if (sent_bytes < 0)
		{
			result.error = WriteResult::UNKNOWN;
			return result;
		}

		result.error = WriteResult::NO_ERROR;
		result.sent_bytes = sent_bytes;
		return result;
	}

	//-----------------------------------------------------------------------------
	bool is_open()
	{
		return m_socket != 0;
	}

public:

	int m_socket;
};

} // namespace crown
