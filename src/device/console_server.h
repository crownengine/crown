/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "socket.h"

namespace crown
{
/// Provides service to communicate with engine via TCP/IP.
///
/// @ingroup Device
class ConsoleServer
{
	TCPSocket _server;
	Vector<TCPSocket> _clients;

	void add_client(TCPSocket socket);
	ReadResult update_client(TCPSocket client);

	void send(TCPSocket client, const char* json);
	void error(TCPSocket client, const char* msg);
	void success(TCPSocket client, const char* msg);
	void process(TCPSocket client, const char* json);

public:

	ConsoleServer(Allocator& a);

	/// Listens on the given @a port. If @a wait is true, this function
	/// blocks until a client is connected.
	void init(u16 port, bool wait);

	/// Shutdowns the server.
	void shutdown();

	/// Collects requests from clients and processes them all.
	void update();

	/// Sends the given JSON-encoded string to all clients.
	void send(const char* json);
};

/// Functions for accessing global console.
namespace console_server_globals
{
	// Creates the global console server.
	void init(u16 port, bool wait);

	/// Destroys the global console server.
	void shutdown();

	/// Updates the global console server.
	void update();

	/// Returns the global console server object.
	ConsoleServer* console();
} // namespace console_server_globals
} // namespace crown
