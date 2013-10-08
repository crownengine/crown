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

#include <winsock2.h>
#pragma comment(lib, "wsock32.lib")

#include "NetAddress.h"
#include "Assert.h"
#include "Types.h"
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

//-----------------------------------------------------------------------------
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
	TCPSocket::~TCPSocket()
	{
		close();
	}

/*	//-----------------------------------------------------------------------------
	bool TCPSocket::open(uint16_t port)
	{
		int32_t sd = socket(AF_INET, SOCK_STREAM, 0);

		if (sd <= 0)
		{
			os::print32_tf("failed to open socket\n");
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
			os::print32_tf("failed to bind socket\n");
			close();
			return false;
		}

		listen(sd, 5);
		os::print32_tf("listening on port %d", port);

		sockaddr_in client;
		uint32_t client_length = sizeof(client);

		int32_t active_sd = accept(sd, (sockaddr*)&client, &client_length);
		if (active_sd < 0)
		{
			os::print32_tf("failed to accept connections");
		}

		set_active_socket_id(active_sd);

		return true;  
	}*/

	//-----------------------------------------------------------------------------
	bool TCPSocket::connect(const NetAddress& destination, uint16_t port)
	{		
		int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (sd <= 0)
		{
			os::printf("failed to open socket\n");
			m_socket = 0;
			return false;
		}
		
		m_socket = sd;

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(destination.address());
		address.sin_port = htons(port);

		if (::connect(sd, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
		{
			os::printf("failed to connect socket\n");
			close();
			return false;
		}
	}

	//-----------------------------------------------------------------------------
	int32_t	TCPSocket::close()
	{
		if (m_socket != 0)
		{
			::close(m_socket);
			m_socket = 0;
		}
	}

	//-----------------------------------------------------------------------------
	ReadResult TCPSocket::read(void* data, int32_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		int received_bytes = recv(m_socket, (char*)data, size, 0);

		ReadResult result;

		if (received_bytes < 0)
		{
			result.error = ReadResult::NO_ERROR;
			result.received_bytes = 0;
		}
		if (received_bytes == 0)
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
	bool TCPSocket::write(const void* data, int32_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		int sent_bytes = ::send(sd, (const char*)data, size, 0);

		WriteResult result;

		if (sent_bytes < 0)
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

//-----------------------------------------------------------------------------
class TCPListener
{
public:

	//-----------------------------------------------------------------------------
	bool open(uint16_t port)
	{
		int& sock_id = m_listener.m_socket;

		sock_id = socket(AF_INET, SOCK_STREAM, 0);
		CE_ASSERT(socket_id != INVALID_SOCKET, "Unable to open socket");

		// set non-blocking io
		DWORD non_blocking = 1;
		int result = ioctlsocket(sock_id, FIONBIO, &non_blocking);
		CE_ASSERT(result == 0, "Unable to set socket non-blocking");

		// Bind socket
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);

		result = ::bind(sock_id, (const sockaddr*)&address, sizeof(sockaddr_in));
		CE_ASSERT(result == 0, "Unable to bind socket");
		
		result = ::listen(sd, 5);
		CE_ASSERT(result == 0, "Unable to listen on socket");

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
		uint32_t client_length = sizeof(client);

		int32_t asd = accept(sd, (sockaddr*)&client, &client_length);
		if (asd < 0)
		{
			return false;
		}

		int result = ioctlsocket(asd, FIONBIO, &non_blocking);
		CE_ASSERT(result == 0, "Unable to set socket non-blocking");		

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

//-----------------------------------------------------------------------------
class UDPSocket
{
	//-----------------------------------------------------------------------------
	UDPSocket::UDPSocket()
		: m_socket(0)
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
		CE_ASSERT(!is_open(), "Socket is already open");

		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		CE_ASSERT(m_socket != INVALID_SOCKET, "Unable to open socket");

		// bind to port
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		int result = ::bind(m_socket, (const sockaddr*)&address, sizeof(sockaddr_in));
		CE_ASSERT(result == 0, "Unable to bind socket");

		// set non-blocking io
		DWORD non_blocking = 1;
		result = ioctlsocket(socket, FIONBIO, &non_blocking);
		CE_ASSERT(result == 0, "Unable to bind socket");

		return true;
	}

	//-----------------------------------------------------------------------------
	ReadResult UDPSocket::read(NetAddress& sender, uint16_t& port, const void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		typedef int socklen_t;

		sockaddr_in from;
		socklen_t from_length = sizeof(from);

		int received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &from_length);

		ReadResult result;

		if (received_bytes < 0)
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
	WriteResult UDPSocket::write(const NetAddress& receiver, uint16_t port, void* data, size_t size)
	{
		CE_ASSERT_NOT_NULL(data);

		if (m_socket == 0)
		{
			return false;
		}

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(receiver.get_address());
		address.sin_port = htons(receiver.get_port());

		int32_t sent_bytes = sendto(m_socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));

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

};

} // namespace crown