/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "socket.h"

namespace crown
{

class ConsoleServer
{
public:

	/// Listens on the given @a port. If @a wait is true, this function
	/// blocks until a client is connected.
	ConsoleServer(uint16_t port, bool wait);
	void shutdown();

	/// Collects requests from clients and processes them all.
	void update();

	/// Sends the given JSON-encoded string to all clients.
	void send(const char* json);

private:

	void send(TCPSocket client, const char* json);

	void add_client(TCPSocket socket);
	ReadResult update_client(TCPSocket client);
	void process(TCPSocket client, const char* json);

	void process_ping(TCPSocket client, const char* json);
	void process_script(TCPSocket client, const char* json);
	void process_command(TCPSocket client, const char* json);

private:

	TCPSocket _server;
	Vector<TCPSocket> _clients;
};

/// Functions for accessing global console.
namespace console_server_globals
{
	// Creates the global console server.
	void init(uint16_t port, bool wait);

	/// Destroys the global console server.
	void shutdown();

	/// Updates the global console server.
	void update();

	/// Returns the global console server object.
	ConsoleServer* console();
} // namespace console_server_globals
} // namespace crown
