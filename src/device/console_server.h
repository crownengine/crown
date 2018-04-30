/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/hash_map.h"
#include "core/containers/types.h"
#include "core/network/socket.h"
#include "core/strings/types.h"

namespace crown
{
/// Provides service to communicate with engine via TCP/IP.
///
/// @ingroup Device
struct ConsoleServer
{
	typedef void (*CommandFunction)(ConsoleServer& cs, TCPSocket client, const char* json, void* user_data);

	struct Command
	{
		CommandFunction function;
		void* user_data;
	};

	TCPSocket _server;
	Array<TCPSocket> _clients;
	HashMap<StringId32, Command> _commands;

	/// Constructor.
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

	/// Sends the given JSON-encoded string to @a client.
	void send(TCPSocket client, const char* json);

	/// Sends an error message to @a client.
	void error(TCPSocket client, const char* msg);

	/// Sends a success message to @a client.
	void success(TCPSocket client, const char* msg);

	/// Registers the command @a type.
	void register_command(const char* type, CommandFunction cmd, void* user_data);
};

namespace console_server_globals
{
	void init();

	void shutdown();

} // namespace console_server_globals

ConsoleServer* console_server();

} // namespace crown
