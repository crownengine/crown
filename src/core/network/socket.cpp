/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/network/ip_address.inl"
#include "core/network/socket.h"
#include "core/platform.h"
#include <new>
#include <string.h> // memcpy

#if CROWN_PLATFORM_POSIX
	#include <errno.h>
	#include <fcntl.h>      // fcntl
	#include <netinet/in.h> // htons, htonl, ...
	#include <sys/socket.h>
	#include <unistd.h>     // close
	typedef int SOCKET;
	#define INVALID_SOCKET (-1)
	#define SOCKET_ERROR (-1)
	#define closesocket close
	#define WSAEADDRINUSE EADDRINUSE
	#define WSAECONNREFUSED ECONNREFUSED
	#define WSAETIMEDOUT ETIMEDOUT
	#define WSAEWOULDBLOCK EWOULDBLOCK
#elif CROWN_PLATFORM_WINDOWS
	#include <winsock2.h>
	#define MSG_NOSIGNAL 0
#endif // CROWN_PLATFORM_POSIX

namespace crown
{
namespace
{
	inline int last_error()
	{
#if CROWN_PLATFORM_POSIX
		return errno;
#elif CROWN_PLATFORM_WINDOWS
		return WSAGetLastError();
#endif
	}

}

struct Private
{
	SOCKET socket;
};

namespace socket_internal
{
	SOCKET open()
	{
		SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CE_ASSERT(socket >= 0, "socket: last_error() = %d", last_error());
		return socket;
	}

	AcceptResult accept(SOCKET socket, TCPSocket& c)
	{
		SOCKET err = ::accept(socket, NULL, NULL);

		AcceptResult ar;
		ar.error = AcceptResult::SUCCESS;

		if (err == INVALID_SOCKET)
		{
			if (last_error() == WSAEWOULDBLOCK)
				ar.error = AcceptResult::NO_CONNECTION;
			else
				ar.error = AcceptResult::UNKNOWN;
		}
		else
		{
			c._priv->socket = (SOCKET)err;
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
				if (last_error() == WSAEWOULDBLOCK)
					rr.error = ReadResult::WOULDBLOCK;
				else if (last_error() == WSAETIMEDOUT)
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
				, MSG_NOSIGNAL // Don't generate SIGPIPE, return EPIPE instead.
				);

			if (bytes_wrote == SOCKET_ERROR)
			{
				if (last_error() == WSAEWOULDBLOCK)
					wr.error = WriteResult::WOULDBLOCK;
				else if (last_error() == WSAETIMEDOUT)
					wr.error = WriteResult::TIMEOUT;
				else if (last_error() == EPIPE)
					wr.error = WriteResult::PIPE;
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

	void set_blocking(SOCKET socket, bool blocking)
	{
#if CROWN_PLATFORM_POSIX
		int flags = fcntl(socket, F_GETFL, 0);
		fcntl(socket, F_SETFL, blocking ? (flags & ~O_NONBLOCK) : O_NONBLOCK);
#elif CROWN_PLATFORM_WINDOWS
		u_long non_blocking = blocking ? 0 : 1;
		int err = ioctlsocket(socket, FIONBIO, &non_blocking);
		CE_ASSERT(err == 0, "ioctlsocket: last_error() = %d", last_error());
		CE_UNUSED(err);
#endif
	}

	void set_reuse_address(SOCKET socket, bool reuse)
	{
		int optval = (int)reuse;
		int err = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
		CE_ASSERT(err == 0, "setsockopt: last_error() = %d", last_error());
		CE_UNUSED(err);
	}

	void set_timeout(SOCKET socket, u32 ms)
	{
		struct timeval tv;
		tv.tv_sec  = ms / 1000;
		tv.tv_usec = ms % 1000 * 1000;

		int err = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
		CE_ASSERT(err == 0, "setsockopt: last_error(): %d", last_error());
		err = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
		CE_ASSERT(err == 0, "setsockopt: last_error(): %d", last_error());
		CE_UNUSED(err);
	}

} // namespace socket_internal

TCPSocket::TCPSocket()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(*_priv));
	_priv = new (_data) Private();
	_priv->socket = INVALID_SOCKET;
}

TCPSocket::TCPSocket(const TCPSocket& other)
{
	_priv = new (_data) Private();
	memcpy(_data, other._data, sizeof(_data));
}

TCPSocket& TCPSocket::operator=(const TCPSocket& other)
{
	_priv = new (_data) Private();
	memcpy(_data, other._data, sizeof(_data));
	return *this;
}

TCPSocket::~TCPSocket()
{
	_priv->~Private();
}

void TCPSocket::close()
{
	if (_priv->socket != INVALID_SOCKET)
	{
		::closesocket(_priv->socket);
		_priv->socket = INVALID_SOCKET;
	}
}

ConnectResult TCPSocket::connect(const IPAddress& ip, u16 port)
{
	close();
	_priv->socket = socket_internal::open();

	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = htonl(ip.address());
	addr_in.sin_port = htons(port);

	int err = ::connect(_priv->socket, (const sockaddr*)&addr_in, sizeof(sockaddr_in));

	ConnectResult cr;
	cr.error = ConnectResult::SUCCESS;

	if (err == SOCKET_ERROR)
	{
		if (last_error() == WSAECONNREFUSED)
			cr.error = ConnectResult::REFUSED;
		else if (last_error() == WSAETIMEDOUT)
			cr.error = ConnectResult::TIMEOUT;
		else
			cr.error = ConnectResult::UNKNOWN;
	}

	return cr;
}

BindResult TCPSocket::bind(u16 port)
{
	close();
	_priv->socket = socket_internal::open();
	socket_internal::set_reuse_address(_priv->socket, true);

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	int err = ::bind(_priv->socket, (const sockaddr*)&address, sizeof(sockaddr_in));

	BindResult br;
	br.error = BindResult::SUCCESS;

	if (err == SOCKET_ERROR)
	{
		if (last_error() == WSAEADDRINUSE)
			br.error = BindResult::ADDRESS_IN_USE;
		else
			br.error = BindResult::UNKNOWN;
	}

	return br;
}

void TCPSocket::listen(u32 max)
{
	int err = ::listen(_priv->socket, max);
	CE_ASSERT(err == 0, "listen: last_error() = %d", last_error());
	CE_UNUSED(err);
}

AcceptResult TCPSocket::accept(TCPSocket& c)
{
	socket_internal::set_blocking(_priv->socket, true);
	return socket_internal::accept(_priv->socket, c);
}

AcceptResult TCPSocket::accept_nonblock(TCPSocket& c)
{
	socket_internal::set_blocking(_priv->socket, false);
	return socket_internal::accept(_priv->socket, c);
}

ReadResult TCPSocket::read(void* data, u32 size)
{
	socket_internal::set_blocking(_priv->socket, true);
	return socket_internal::read(_priv->socket, data, size);
}

ReadResult TCPSocket::read_nonblock(void* data, u32 size)
{
	socket_internal::set_blocking(_priv->socket, false);
	return socket_internal::read(_priv->socket, data, size);
}

WriteResult TCPSocket::write(const void* data, u32 size)
{
	socket_internal::set_blocking(_priv->socket, true);
	return socket_internal::write(_priv->socket, data, size);
}

WriteResult TCPSocket::write_nonblock(const void* data, u32 size)
{
	socket_internal::set_blocking(_priv->socket, false);
	return socket_internal::write(_priv->socket, data, size);
}

} // namespace crown
