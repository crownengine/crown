/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "types.h"
#include "error.h"
#include "macros.h"
#include "net_address.h"

#if CROWN_PLATFORM_POSIX
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <winsock2.h>
	#include "win_headers.h"
	#pragma comment(lib, "Ws2_32.lib")
#endif

namespace crown
{
struct ConnectResult
{
	enum { NO_ERROR, BAD_SOCKET, REFUSED, TIMEOUT, UNKNOWN } error;
};

struct ReadResult
{
	enum { NO_ERROR, BAD_SOCKET, REMOTE_CLOSED, TIMEOUT, UNKNOWN } error;
	u32 bytes_read;
};

struct WriteResult
{
	enum { NO_ERROR, BAD_SOCKET, REMOTE_CLOSED, TIMEOUT, UNKNOWN } error;
	u32 bytes_wrote;
};

struct AcceptResult
{
	enum { NO_ERROR, BAD_SOCKET, NO_CONNECTION, UNKNOWN } error;
};

/// TCP socket
///
/// @ingroup Network
struct TCPSocket
{
	TCPSocket()
#if CROWN_PLATFORM_POSIX
		: _socket(0)
#elif CROWN_PLATFORM_WINDOWS
		: _socket(INVALID_SOCKET)
#endif
	{
	}

	void open()
	{
#if CROWN_PLATFORM_POSIX
		_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CE_ASSERT(_socket >= 0, "socket: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CE_ASSERT(_socket >= 0, "socket: WSAGetLastError = %d", WSAGetLastError());
#endif
	}

	ConnectResult connect(const NetAddress& ip, u16 port)
	{
		close();
		open();

		ConnectResult cr;
		cr.error = ConnectResult::NO_ERROR;

#if CROWN_PLATFORM_POSIX
		sockaddr_in addr_in;
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.s_addr = htonl(ip.address());
		addr_in.sin_port = htons(port);

		int err = ::connect(_socket, (const sockaddr*)&addr_in, sizeof(sockaddr_in));

		if (err == 0)
			return cr;

		if (errno == ECONNREFUSED)
			cr.error = ConnectResult::REFUSED;
		else if (errno == ETIMEDOUT)
			cr.error = ConnectResult::TIMEOUT;
		else
			cr.error = ConnectResult::UNKNOWN;

		return cr;
#elif CROWN_PLATFORM_WINDOWS
		sockaddr_in addr_in;
		addr_in.sin_family = AF_INET;
		addr_in.sin_addr.s_addr = ::htonl(ip.address());
		addr_in.sin_port = ::htons(port);

		int err = ::connect(_socket, (const sockaddr*)&addr_in, sizeof(sockaddr_in));

		if (err == 0)
			return cr;

		int wsaerr = WSAGetLastError();
		if (wsaerr == WSAECONNREFUSED)
			cr.error = ConnectResult::REFUSED;
		else if (wsaerr == WSAETIMEDOUT)
			cr.error = ConnectResult::TIMEOUT;
		else
			cr.error = ConnectResult::UNKNOWN;

		return cr;
#endif
	}

	bool bind(u16 port)
	{
		close();
		open();
		set_reuse_address(true);
#if CROWN_PLATFORM_POSIX
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);

		int err = ::bind(_socket, (const sockaddr*)&address, sizeof(sockaddr_in));
		CE_ASSERT(err == 0, "bind: errno = %d", errno);
		CE_UNUSED(err);
		return true;
#elif CROWN_PLATFORM_WINDOWS
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);

		int err = ::bind(_socket, (const sockaddr*)&address, sizeof(sockaddr_in));
		CE_ASSERT(err == 0, "bind: WSAGetLastError = %d", WSAGetLastError());
		CE_UNUSED(err);
		return true;
#endif
	}

	void listen(u32 max)
	{
#if CROWN_PLATFORM_POSIX
		int err = ::listen(_socket, max);
		CE_ASSERT(err == 0, "listen: errno = %d", errno);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		int err = ::listen(_socket, max);
		CE_ASSERT(err == 0, "listen: WSAGetLastError = %d", WSAGetLastError());
		CE_UNUSED(err);
#endif
	}

	AcceptResult accept_internal(TCPSocket& c)
	{
		AcceptResult ar;
		ar.error = AcceptResult::NO_ERROR;

#if CROWN_PLATFORM_POSIX
		int err = ::accept(_socket, NULL, NULL);

		if (err >= 0)
			c._socket = err;
		else if (err == -1 && errno == EBADF)
			ar.error = AcceptResult::BAD_SOCKET;
		else if (err == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			ar.error = AcceptResult::NO_CONNECTION;
		else
			ar.error = AcceptResult::UNKNOWN;

		return ar;
#elif CROWN_PLATFORM_WINDOWS
		SOCKET err = ::accept(_socket, NULL, NULL);

		if (err != INVALID_SOCKET)
		{
			c._socket = err;
			return ar;
		}

		int wsaerr = WSAGetLastError();
		if (wsaerr == WSAEWOULDBLOCK)
			ar.error = AcceptResult::NO_CONNECTION;
		else
			ar.error = AcceptResult::UNKNOWN;

		return ar;
#endif
	}

	AcceptResult accept_nonblock(TCPSocket& c)
	{
		set_blocking(false);
		return accept_internal(c);
	}

	AcceptResult accept(TCPSocket& c)
	{
		set_blocking(true);
		return accept_internal(c);
	}

	void close()
	{
#if CROWN_PLATFORM_POSIX
		if (_socket != 0)
		{
			::close(_socket);
			_socket = 0;
		}
#elif CROWN_PLATFORM_WINDOWS
		if (_socket != INVALID_SOCKET)
		{
			::closesocket(_socket);
			_socket = INVALID_SOCKET;
		}
#endif
	}

	ReadResult read_internal(void* data, u32 size)
	{
		ReadResult rr;
		rr.error = ReadResult::NO_ERROR;
		rr.bytes_read = 0;

#if CROWN_PLATFORM_POSIX
		char* buf = (char*)data;
		u32 to_read = size;

		while (to_read > 0)
		{
			ssize_t read_bytes = ::recv(_socket, buf, to_read, 0);

			if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				return rr;
			}
			else if (read_bytes == -1 && errno == ETIMEDOUT)
			{
				rr.error = ReadResult::TIMEOUT;
				return rr;
			}
			else if (read_bytes == 0)
			{
				rr.error = ReadResult::REMOTE_CLOSED;
				return rr;
			}

			buf += read_bytes;
			to_read -= read_bytes;
			rr.bytes_read += read_bytes;
		}

		return rr;
#elif CROWN_PLATFORM_WINDOWS
		char* buf = (char*)data;
		u32 to_read = size;

		while (to_read > 0)
		{
			int read_bytes = ::recv(_socket, buf, (int)to_read, 0);

			if (read_bytes == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
				return rr;
			else if (read_bytes == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
			{
				rr.error = ReadResult::TIMEOUT;
				return rr;
			}
			else if (read_bytes == 0)
			{
				rr.error = ReadResult::REMOTE_CLOSED;
				return rr;
			}

			buf += read_bytes;
			to_read -= read_bytes;
			rr.bytes_read += read_bytes;
		}

		return rr;
#endif
	}

	ReadResult read_nonblock(void* data, u32 size)
	{
		set_blocking(false);
		return read_internal(data, size);
	}

	ReadResult read(void* data, u32 size)
	{
		set_blocking(true);
		return read_internal(data, size);
	}

	WriteResult write_internal(const void* data, u32 size)
	{
		WriteResult wr;
		wr.error = WriteResult::NO_ERROR;
		wr.bytes_wrote = 0;

#if CROWN_PLATFORM_POSIX
		const char* buf = (const char*)data;
		u32 to_send = size;

		while (to_send > 0)
		{
			ssize_t bytes_wrote = ::send(_socket, (const char*)buf, to_send, 0);

			if (bytes_wrote == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
				return wr;
			else if (bytes_wrote == -1 && errno == ETIMEDOUT)
			{
				wr.error = WriteResult::TIMEOUT;
				return wr;
			}
			else if (bytes_wrote == 0)
			{
				wr.error = WriteResult::REMOTE_CLOSED;
				return wr;
			}
			else
			{
				wr.error = WriteResult::UNKNOWN;
				return wr;
			}

			buf += bytes_wrote;
			to_send -= bytes_wrote;
			wr.bytes_wrote += bytes_wrote;
		}

		return wr;
#elif CROWN_PLATFORM_WINDOWS
		const char* buf = (const char*)data;
		u32 to_send = size;

		while (to_send > 0)
		{
			int bytes_wrote = ::send(_socket, (const char*)buf, (int)to_send, 0);

			if (bytes_wrote == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
			{
				return wr;
			}
			else if (bytes_wrote == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT)
			{
				wr.error = WriteResult::TIMEOUT;
				return wr;
			}
			else if (bytes_wrote == 0)
			{
				wr.error = WriteResult::REMOTE_CLOSED;
				return wr;
			}
			else
			{
				wr.error = WriteResult::UNKNOWN;
				return wr;
			}

			buf += bytes_wrote;
			to_send -= bytes_wrote;
			wr.bytes_wrote += bytes_wrote;
		}

		return wr;
#endif
	}

	WriteResult write_nonblock(const void* data, u32 size)
	{
		set_blocking(false);
		return write_internal(data, size);
	}

	WriteResult write(const void* data, u32 size)
	{
		set_blocking(true);
		return write_internal(data, size);
	}

	void set_blocking(bool blocking)
	{
#if CROWN_PLATFORM_POSIX
		int flags = fcntl(_socket, F_GETFL, 0);
		fcntl(_socket, F_SETFL, blocking ? (flags & ~O_NONBLOCK) : O_NONBLOCK);
#elif CROWN_PLATFORM_WINDOWS
		//Warning! http://www.sockets.com/winsock.htm#IoctlSocket
		u_long non_blocking = blocking ? 0 : 1;
		ioctlsocket(_socket, FIONBIO, &non_blocking);
#endif
	}

	void set_reuse_address(bool reuse)
	{
#if CROWN_PLATFORM_POSIX
		int optval = (int)reuse;
		int err = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
		CE_ASSERT(err == 0, "setsockopt: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		int optval = (int)reuse;
		int err = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
		CE_ASSERT(err == 0, "setsockopt: WSAGetLastError = %d", WSAGetLastError());
#endif
		CE_UNUSED(err);
	}

	void set_timeout(u32 seconds)
	{
#if CROWN_PLATFORM_POSIX
		struct timeval timeout;
		timeout.tv_sec = seconds;
		timeout.tv_usec = 0;
		int err = setsockopt (_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		CE_ASSERT(err == 0, "setsockopt: errno: %d", errno);
		err = setsockopt (_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		CE_ASSERT(err == 0, "setsockopt: errno: %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		struct timeval timeout;
		timeout.tv_sec = seconds;
		timeout.tv_usec = 0;
		int err = setsockopt (_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		CE_ASSERT(err == 0, "setsockopt: WSAGetLastError: %d", WSAGetLastError());
		err = setsockopt (_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		CE_ASSERT(err == 0, "setsockopt: WSAGetLastError: %d", WSAGetLastError());
#endif
		CE_UNUSED(err);
	}

private:

#if CROWN_PLATFORM_POSIX
	int _socket;
#elif CROWN_PLATFORM_WINDOWS
	SOCKET _socket;
#endif
};

} // namespace crown
