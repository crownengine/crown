/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "ip_address.h"
#include "socket.h"

#if CROWN_PLATFORM_POSIX
	#include <errno.h>
	#include <fcntl.h>      // fcntl
	#include <netinet/in.h> // htons, htonl, ...
	#include <sys/socket.h>
	#include <unistd.h>     // close
	#define INVALID_SOCKET (-1)
	#define SOCKET_ERROR (-1)
	#define closesocket close
#elif CROWN_PLATFORM_WINDOWS
	#include <winsock2.h>
	#pragma comment(lib, "Ws2_32.lib")
	#ifndef _INC_ERRNO
		#define EADDRINUSE WSAEADDRINUSE
		#define ECONNREFUSED WSAECONNREFUSED
		#define ETIMEDOUT WSAETIMEDOUT
		#define EWOULDBLOCK WSAEWOULDBLOCK
	#endif // _INC_ERRNO
#endif

namespace crown
{
namespace
{
	inline int last_error()
	{
#ifdef CROWN_PLATFORM_LINUX
		return errno;
#elif CROWN_PLATFORM_WINDOWS
		return WSAGetLastError();
#endif
	}
}

namespace socket_internal
{
	void open(SOCKET socket)
	{
		socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CE_ASSERT(socket >= 0, "socket: last_error() = %d", last_error());
	}

	AcceptResult accept(SOCKET socket, TCPSocket& c)
	{
		SOCKET err = ::accept(socket, NULL, NULL);

		AcceptResult ar;
		ar.error = AcceptResult::SUCCESS;

		if (err == INVALID_SOCKET)
		{
			if (last_error() == EWOULDBLOCK)
				ar.error = AcceptResult::NO_CONNECTION;
			else
				ar.error = AcceptResult::UNKNOWN;
		}
		else
		{
			c._socket = err;
		}

		return ar;
	}

	ReadResult read(SOCKET socket, void* data, u32 size)
	{
		ReadResult rr;
		rr.error = ReadResult::SUCCESS;
		rr.bytes_read = 0;

		u32 to_read = size;

		while (to_read > 0)
		{
			int bytes_read = ::recv(socket
				, (char*)data + rr.bytes_read
				, to_read
				, 0
				);

			if (bytes_read == SOCKET_ERROR)
			{
				if (last_error() == EWOULDBLOCK)
					rr.error = ReadResult::WOULDBLOCK;
				else if (last_error() == ETIMEDOUT)
					rr.error = ReadResult::TIMEOUT;
				else
					rr.error = ReadResult::UNKNOWN;
				return rr;
			}
			else if (bytes_read == 0)
			{
				rr.error = ReadResult::REMOTE_CLOSED;
				return rr;
			}

			to_read -= bytes_read;
			rr.bytes_read += bytes_read;
		}

		return rr;
	}

	WriteResult write(SOCKET socket, const void* data, u32 size)
	{
		WriteResult wr;
		wr.error = WriteResult::SUCCESS;
		wr.bytes_wrote = 0;

		u32 to_write = size;

		while (to_write > 0)
		{
			int bytes_wrote = ::send(socket
				, (char*)data + wr.bytes_wrote
				, to_write
				, 0
				);

			if (bytes_wrote == SOCKET_ERROR)
			{
				if (last_error() == EWOULDBLOCK)
					wr.error = WriteResult::WOULDBLOCK;
				else if (last_error() == ETIMEDOUT)
					wr.error = WriteResult::TIMEOUT;
				else
					wr.error = WriteResult::UNKNOWN;
				return wr;
			}
			else if (bytes_wrote == 0)
			{
				wr.error = WriteResult::REMOTE_CLOSED;
				return wr;
			}

			to_write -= bytes_wrote;
			wr.bytes_wrote += bytes_wrote;
		}
		return wr;
	}

} // namespace socket_internal

TCPSocket::TCPSocket()
	: _socket(INVALID_SOCKET)
{
}

void TCPSocket::close()
{
	if (_socket != INVALID_SOCKET)
	{
		::closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

ConnectResult TCPSocket::connect(const IPAddress& ip, u16 port)
{
	close();
	socket_internal::open(_socket);

	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = htonl(ip.address());
	addr_in.sin_port = htons(port);

	int err = ::connect(_socket, (const sockaddr*)&addr_in, sizeof(sockaddr_in));

	ConnectResult cr;
	cr.error = ConnectResult::SUCCESS;

	if (err == SOCKET_ERROR)
	{
		if (last_error() == ECONNREFUSED)
			cr.error = ConnectResult::REFUSED;
		else if (last_error() == ETIMEDOUT)
			cr.error = ConnectResult::TIMEOUT;
		else
			cr.error = ConnectResult::UNKNOWN;
	}

	return cr;
}

BindResult TCPSocket::bind(u16 port)
{
	close();
	socket_internal::open(_socket);
	set_reuse_address(true);

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	int err = ::bind(_socket, (const sockaddr*)&address, sizeof(sockaddr_in));

	BindResult br;
	br.error = BindResult::SUCCESS;

	if (err == SOCKET_ERROR)
	{
		if (last_error() == EADDRINUSE)
			br.error = BindResult::ADDRESS_IN_USE;
		else
			br.error = BindResult::UNKNOWN;
	}

	return br;
}

void TCPSocket::listen(u32 max)
{
	int err = ::listen(_socket, max);
	CE_ASSERT(err == 0, "listen: last_error() = %d", last_error());
	CE_UNUSED(err);
}

AcceptResult TCPSocket::accept(TCPSocket& c)
{
	set_blocking(true);
	return socket_internal::accept(_socket, c);
}

AcceptResult TCPSocket::accept_nonblock(TCPSocket& c)
{
	set_blocking(false);
	return socket_internal::accept(_socket, c);
}

ReadResult TCPSocket::read(void* data, u32 size)
{
	set_blocking(true);
	return socket_internal::read(_socket, data, size);
}

ReadResult TCPSocket::read_nonblock(void* data, u32 size)
{
	set_blocking(false);
	return socket_internal::read(_socket, data, size);
}

WriteResult TCPSocket::write(const void* data, u32 size)
{
	set_blocking(true);
	return socket_internal::write(_socket, data, size);
}

WriteResult TCPSocket::write_nonblock(const void* data, u32 size)
{
	set_blocking(false);
	return socket_internal::write(_socket, data, size);
}

void TCPSocket::set_blocking(bool blocking)
{
#if CROWN_PLATFORM_POSIX
	int flags = fcntl(_socket, F_GETFL, 0);
	fcntl(_socket, F_SETFL, blocking ? (flags & ~O_NONBLOCK) : O_NONBLOCK);
#elif CROWN_PLATFORM_WINDOWS
	u_long non_blocking = blocking ? 0 : 1;
	ioctlsocket(_socket, FIONBIO, &non_blocking);
#endif
}

void TCPSocket::set_reuse_address(bool reuse)
{
	int optval = (int)reuse;
	int err = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	CE_ASSERT(err == 0, "setsockopt: last_error() = %d", last_error());
	CE_UNUSED(err);
}

void TCPSocket::set_timeout(u32 seconds)
{
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;

	int err = setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	CE_ASSERT(err == 0, "setsockopt: last_error(): %d", last_error());
	err = setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
	CE_ASSERT(err == 0, "setsockopt: last_error(): %d", last_error());
	CE_UNUSED(err);
}

} // namespace crown
