/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "socket.h"
#include "string_types.h"

namespace crown
{
/// Provides service to communicate with engine via TCP/IP.
///
/// @ingroup Device
class ConsoleServer
{
	typedef void (*CommandFunction)(void* data, ConsoleServer& cs, TCPSocket client, const char* json);

	struct CommandData
	{
		CommandFunction cmd;
		void* data;
	};

	TCPSocket _server;
	Vector<TCPSocket> _clients;
	SortMap<StringId32, CommandData> _commands;

	void add_client(TCPSocket socket);
	ReadResult update_client(TCPSocket client);

	void send(TCPSocket client, const char* json);
	void process(TCPSocket client, const char* json);

public:

	ConsoleServer(Allocator& a);

	/// Listens on the given @a port. If @a wait is true, this function
	/// blocks until a client is connected.
	void listen(u16 port, bool wait);

	/// Shutdowns the server.
	void shutdown();

	/// Collects requests from clients and processes them all.
	void update();

	/// Sends the given JSON-encoded string to all clients.
	void send(const char* json);

	/// Sends an error message to @a client.
	void error(TCPSocket client, const char* msg);

	/// Sends a success message to @a client.
	void success(TCPSocket client, const char* msg);

	/// Registers the command @a type.
	void register_command(StringId32 type, CommandFunction cmd, void* data);
};

} // namespace crown
