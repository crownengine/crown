/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/json/types.h"
#include "core/network/socket.h"
#include "core/strings/types.h"
#include "core/thread/condition_variable.h"
#include "core/thread/mutex.h"
#include "core/thread/semaphore.h"
#include "core/thread/thread.h"

namespace crown
{
/// Provides service to communicate with engine via TCP/IP.
///
/// @ingroup Device
struct ConsoleServer
{
	typedef void (*CommandTypeFunction)(ConsoleServer& cs, u32 client_id, JsonArray& args, void* user_data);
	typedef void (*MessageTypeFunction)(ConsoleServer& cs, u32 client_id, const char* json, void* user_data);

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

	u16 _port;
	TCPSocket _server;
	u32 _next_client_id;
	Mutex _clients_mutex;
	Vector<Client> _clients;
	HashMap<StringId32, CommandData> _messages;
	HashMap<StringId32, CommandData> _commands;

	bool _thread_exit;

	Thread _input_thread;
	Buffer _input_0;
	Buffer _input_1;
	Buffer* _input_write;
	Buffer* _input_read;
	Semaphore _input_semaphore;
	Semaphore _handlers_semaphore;

	Thread _output_thread;
	Mutex _output_mutex;
	ConditionVariable _output_condition;
	Buffer _output_0;
	Buffer _output_1;
	Buffer* _output_write;
	Buffer* _output_read;

	SocketSet _read_socket_set;
	SocketSet _active_socket_set;
	Semaphore _client_connected;

	///
	ConsoleServer(Allocator& a);

	/// Listens on the given @a port. If @a wait is true, this function
	/// blocks until a client is connected.
	void listen(u16 port, bool wait);

	/// Shutdowns the server.
	void shutdown();

	/// Sends the given JSON-encoded string to @a client_id.
	void send(u32 client_id, const char* json);

	/// Sends an error message to @a client.
	void error(u32 client_id, const char* msg);

	/// Sends the given JSON-encoded string to all clients.
	void broadcast(const char* json);

	// Registers the command @a type.
	void register_command_name(const char* name, const char* brief, CommandTypeFunction cmd, void* user_data);

	/// Registers the message @a type.
	void register_message_type(const char* type, MessageTypeFunction cmd, void* user_data);

	/// Executes the message handlers for all the messages successfully received by
	/// the console so far. If @a sync is true, this function blocks until there is
	/// data available to be consumed by message handlers, otherwise it returns
	/// immediately. It must be called on a regular basis to avoid blocking the
	/// input thread.
	void execute_message_handlers(bool sync);

	///
	s32 run_input_thread();

	///
	s32 run_output_thread();
};

namespace console_server_globals
{
	///
	void init();

	///
	void shutdown();

} // namespace console_server_globals

ConsoleServer* console_server();

} // namespace crown
