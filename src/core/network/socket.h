/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "network_types.h"
#include "platform.h"
#include "types.h"

#if CROWN_PLATFORM_POSIX
	typedef int SOCKET;
#elif CROWN_PLATFORM_WINDOWS
	#include <winsock2.h>
	#pragma comment(lib, "Ws2_32.lib")
#endif

namespace crown
{
struct ConnectResult
{
	enum
	{
		SUCCESS,
		REFUSED,
		TIMEOUT,
		UNKNOWN
	} error;
};

struct BindResult
{
	enum
	{
		SUCCESS,
		ADDRESS_IN_USE,
		UNKNOWN
	} error;
};

struct AcceptResult
{
	enum
	{
		SUCCESS,
		NO_CONNECTION,
		UNKNOWN
	} error;
};

struct ReadResult
{
	enum
	{
		SUCCESS,
		WOULDBLOCK,
		REMOTE_CLOSED,
		TIMEOUT,
		UNKNOWN
	} error;
	u32 bytes_read;
};

struct WriteResult
{
	enum
	{
		SUCCESS,
		WOULDBLOCK,
		REMOTE_CLOSED,
		TIMEOUT,
		UNKNOWN
	} error;
	u32 bytes_wrote;
};

/// TCP socket
///
/// @ingroup Network
struct TCPSocket
{
	SOCKET _socket;

	TCPSocket();

	/// Closes the socket.
	void close();

	/// Connects to the @a ip address and @a port and returns the result.
	ConnectResult connect(const IPAddress& ip, u16 port);

	/// Binds the socket to @a port and returns the result.
	BindResult bind(u16 port);

	/// Listens for @a max socket connections.
	void listen(u32 max);

	/// Accepts a new connection @a c.
	AcceptResult accept(TCPSocket& c);

	/// Accepts a new connection @a c.
	AcceptResult accept_nonblock(TCPSocket& c);

	/// Reads @a size bytes and returns the result.
	ReadResult read(void* data, u32 size);

	/// Reads @a size bytes and returns the result.
	ReadResult read_nonblock(void* data, u32 size);

	/// Writes @a size bytes and returns the result.
	WriteResult write(const void* data, u32 size);

	/// Writes @a size bytes and returns the result.
	WriteResult write_nonblock(const void* data, u32 size);

	/// Sets whether the socket is @a blocking.
	void set_blocking(bool blocking);

	/// Sets whether the socket should @a reuse a busy port.
	void set_reuse_address(bool reuse);

	/// Sets the timeout to the given @a seconds.
	void set_timeout(u32 seconds);
};

} // namespace crown
