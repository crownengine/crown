/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/hash_map.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/string_id.h"
#include "core/strings/string_stream.h"
#include "device/console_server.h"

namespace crown
{
ConsoleServer::ConsoleServer(Allocator& a)
	: _clients(a)
	, _commands(a)
{
}

void ConsoleServer::listen(u16 port, bool wait)
{
	_server.bind(port);
	_server.listen(5);

	if (wait)
	{
		AcceptResult ar;
		TCPSocket client;
		do
		{
			ar = _server.accept(client);
		}
		while (ar.error != AcceptResult::SUCCESS);

		array::push_back(_clients, client);
	}
}

void ConsoleServer::shutdown()
{
	for (u32 i = 0; i < array::size(_clients); ++i)
		_clients[i].close();

	_server.close();
}

void ConsoleServer::send(TCPSocket client, const char* json)
{
	u32 len = strlen32(json);
	client.write(&len, 4);
	client.write(json, len);
}

void ConsoleServer::error(TCPSocket client, const char* msg)
{
	TempAllocator4096 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"error\",\"message\":\"" << msg << "\"}";
	send(client, string_stream::c_str(ss));
}

void ConsoleServer::send(const char* json)
{
	for (u32 i = 0; i < array::size(_clients); ++i)
		send(_clients[i], json);
}

void ConsoleServer::update()
{
	TCPSocket client;
	AcceptResult ar = _server.accept_nonblock(client);
	if (ar.error == AcceptResult::SUCCESS)
		array::push_back(_clients, client);

	TempAllocator256 alloc;
	Array<u32> to_remove(alloc);

	// Update all clients
	for (u32 i = 0; i < array::size(_clients); ++i)
	{
		for (;;)
		{
			u32 msg_len = 0;
			ReadResult rr = _clients[i].read_nonblock(&msg_len, 4);

			if (rr.error == ReadResult::WOULDBLOCK)
				break;

			if (rr.error != ReadResult::SUCCESS)
			{
				array::push_back(to_remove, i);
				break;
			}

			// Read message
			TempAllocator4096 ta;
			Array<char> msg(ta);
			array::resize(msg, msg_len + 1);
			rr = _clients[i].read(array::begin(msg), msg_len);
			array::push_back(msg, '\0');

			if (rr.error != ReadResult::SUCCESS)
			{
				array::push_back(to_remove, i);
				break;
			}

			// Process message
			JsonObject obj(ta);
			sjson::parse(array::begin(msg), obj);

			Command cmd;
			cmd.function = NULL;
			cmd.user_data = NULL;
			cmd = hash_map::get(_commands
				, sjson::parse_string_id(obj["type"])
				, cmd
				);

			if (cmd.function)
				cmd.function(*this, _clients[i], array::begin(msg), cmd.user_data);
			else
				error(_clients[i], "Unknown command");
		}
	}

	// Remove clients
	for (u32 i = 0; i < array::size(to_remove); ++i)
	{
		const u32 last = array::size(_clients) - 1;
		const u32 c = to_remove[i];

		_clients[c].close();
		_clients[c] = _clients[last];
		array::pop_back(_clients);
	}
}

void ConsoleServer::register_command(const char* type, CommandFunction function, void* user_data)
{
	CE_ENSURE(NULL != type);
	CE_ENSURE(NULL != function);

	Command cmd;
	cmd.function = function;
	cmd.user_data = user_data;

	hash_map::set(_commands, StringId32(type), cmd);
}

namespace console_server_globals
{
	ConsoleServer* _console_server = NULL;

	void init()
	{
		_console_server = CE_NEW(default_allocator(), ConsoleServer)(default_allocator());
	}

	void shutdown()
	{
		_console_server->shutdown();
		CE_DELETE(default_allocator(), _console_server);
		_console_server = NULL;
	}

} // namespace console_server_globals

ConsoleServer* console_server()
{
	return console_server_globals::_console_server;
}

} // namespace crown
