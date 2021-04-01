/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/json/types.h"
#include "core/network/socket.h"
#include "core/strings/types.h"

namespace crown
{
/// Provides service to communicate with engine via TCP/IP.
///
/// @ingroup Device
struct ConsoleServer
{
	typedef void (*CommandTypeFunction)(ConsoleServer& cs, TCPSocket& client, JsonArray& args, void* user_data);
	typedef void (*MessageTypeFunction)(ConsoleServer& cs, TCPSocket& client, const char* json, void* user_data);

	struct CommandData
	{
		union
		{
			CommandTypeFunction command_function;
			MessageTypeFunction message_function;
		};

		void* user_data;
		char name[32];
		char brief[128];
	};

	struct Client
	{
		TCPSocket socket;
		u32 id;
	};

	TCPSocket _server;
	u32 _next_client_id;
	Vector<Client> _clients;
	HashMap<StringId32, CommandData> _messages;
	HashMap<StringId32, CommandData> _commands;

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
	void send(TCPSocket& client, const char* json);

	/// Sends an error message to @a client.
	void error(TCPSocket& client, const char* msg);

	// Registers the command @a type.
	void register_command_name(const char* name, const char* brief, CommandTypeFunction cmd, void* user_data);

	/// Registers the message @a type.
	void register_message_type(const char* type, MessageTypeFunction cmd, void* user_data);
};

namespace console_server_globals
{
	void init();

	void shutdown();

} // namespace console_server_globals

ConsoleServer* console_server();

} // namespace crown
