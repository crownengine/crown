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
		address.sin_port = htons(port);

		if (::connect(sd, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
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
	ReadResult read(void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		ssize_t received_bytes = ::read(m_socket, (char*) data, size);

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

		return result;
	}

	//-----------------------------------------------------------------------------
	WriteResult write(const void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		ssize_t sent_bytes = ::send(m_socket, (const char*) data, size, 0);

		WriteResult result;

		if (sent_bytes == -1)
		{
			result.error = WriteResult::UNKNOWN;
		}
		else
		{
			result.error = WriteResult::NO_ERROR;
			result.sent_bytes = sent_bytes;
		}

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

		fcntl(asd, F_SETFL, O_NONBLOCK);
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

} // namespace crown
