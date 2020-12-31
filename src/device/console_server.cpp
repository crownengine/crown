/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/vector.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_stream.inl"
#include "device/console_server.h"

LOG_SYSTEM(CONSOLE_SERVER, "console_server")

namespace crown
{
namespace console_server_internal
{
	static void message_command(ConsoleServer& cs, TCPSocket& client, const char* json, void* /*user_data*/)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		JsonArray args(ta);

		sjson::parse(obj, json);
		sjson::parse_array(args, obj["args"]);

		DynamicString command_name(ta);
		sjson::parse_string(command_name, args[0]);

		ConsoleServer::CommandData cmd;
		cmd.command_function = NULL;
		cmd.user_data = NULL;
		cmd = hash_map::get(cs._commands, command_name.to_string_id(), cmd);

		if (cmd.command_function != NULL)
			cmd.command_function(cs, client, args, cmd.user_data);
	}

	static void command_help(ConsoleServer& cs, TCPSocket& client, JsonArray& args, void* /*user_data*/)
	{
		if (array::size(args) != 1)
		{
			cs.error(client, "Usage: help");
			return;
		}

		u32 longest = 0;

		auto cur = hash_map::begin(cs._commands);
		auto end = hash_map::end(cs._commands);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(cs._commands, cur);

			if (longest < strlen32(cur->second.name))
				longest = strlen32(cur->second.name);
		}

		cur = hash_map::begin(cs._commands);
		end = hash_map::end(cs._commands);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(cs._commands, cur);

			logi(CONSOLE_SERVER, "%s%*s%s"
				, cur->second.name
				, longest - strlen32(cur->second.name) + 2
				, " "
				, cur->second.brief
				);
		}
	}

	static u32 add_client(ConsoleServer& cs, const TCPSocket& socket)
	{
		const u32 id = cs._next_client_id++;

		ConsoleServer::Client client;
		client.socket = socket;
		client.id = id;
		vector::push_back(cs._clients, client);

		return id;
	}

	static void remove_client(ConsoleServer& cs, u32 id)
	{
		const u32 last = vector::size(cs._clients) - 1;

		for (u32 cc = 0; cc < vector::size(cs._clients); ++cc)
		{
			if (cs._clients[cc].id == id)
			{
				cs._clients[cc].socket.close();
				cs._clients[cc] = cs._clients[last];
				vector::pop_back(cs._clients);
				return;
			}
		}
	}

} // namespace console_server_internal

ConsoleServer::ConsoleServer(Allocator& a)
	: _next_client_id(0)
	, _clients(a)
	, _messages(a)
	, _commands(a)
{
	this->register_message_type("command", console_server_internal::message_command, this);
	this->register_command_name("help", "List all commands", console_server_internal::command_help, this);
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

		console_server_internal::add_client(*this, client);
	}
}

void ConsoleServer::shutdown()
{
	for (u32 i = 0; i < vector::size(_clients); ++i)
		_clients[i].socket.close();

	_server.close();
}

void ConsoleServer::send(TCPSocket& client, const char* json)
{
	u32 len = strlen32(json);
	client.write(&len, 4);
	client.write(json, len);
}

void ConsoleServer::error(TCPSocket& client, const char* msg)
{
	TempAllocator4096 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"error\",\"message\":\"" << msg << "\"}";
	send(client, string_stream::c_str(ss));
}

void ConsoleServer::log(LogSeverity::Enum sev, const char* system, const char* msg)
{
	const char* severity_map[] = { "info", "warning", "error" };
	CE_STATIC_ASSERT(countof(severity_map) == LogSeverity::COUNT);

	if (vector::size(_clients) == 0)
		return;

	TempAllocator4096 ta;
	StringStream ss(ta);

	ss << "{\"type\":\"message\",\"severity\":\"";
	ss << severity_map[sev];
	ss << "\",\"system\":\"";
	ss << system;
	ss << "\",\"message\":\"";

	// Sanitize msg
	const char* ch = msg;
	for (; *ch; ch++)
	{
		if (*ch == '"' || *ch == '\\')
			ss << "\\";
		ss << *ch;
	}
	ss << "\"}";

	send(string_stream::c_str(ss));
}

void ConsoleServer::send(const char* json)
{
	for (u32 i = 0; i < vector::size(_clients); ++i)
		send(_clients[i].socket, json);
}

void ConsoleServer::update()
{
	TCPSocket client;
	AcceptResult ar = _server.accept_nonblock(client);
	if (ar.error == AcceptResult::SUCCESS)
		console_server_internal::add_client(*this, client);

	TempAllocator256 alloc;
	Array<u32> to_remove(alloc);

	// Update all clients
	for (u32 i = 0; i < vector::size(_clients); ++i)
	{
		for (;;)
		{
			u32 msg_len = 0;
			ReadResult rr = _clients[i].socket.read_nonblock(&msg_len, 4);

			if (rr.error == ReadResult::WOULDBLOCK)
				break;

			if (rr.error != ReadResult::SUCCESS)
			{
				array::push_back(to_remove, _clients[i].id);
				break;
			}

			// Read message
			TempAllocator4096 ta;
			Array<char> msg(ta);
			array::resize(msg, msg_len + 1);
			rr = _clients[i].socket.read(array::begin(msg), msg_len);
			msg[msg_len] = '\0';

			if (rr.error != ReadResult::SUCCESS)
			{
				array::push_back(to_remove, _clients[i].id);
				break;
			}

			// Process message
			JsonObject obj(ta);
			sjson::parse(obj, array::begin(msg));

			CommandData cmd;
			cmd.message_function = NULL;
			cmd.user_data = NULL;
			cmd = hash_map::get(_messages
				, sjson::parse_string_id(obj["type"])
				, cmd
				);

			if (cmd.message_function)
				cmd.message_function(*this, _clients[i].socket, array::begin(msg), cmd.user_data);
			else
				error(_clients[i].socket, "Unknown command");
		}
	}

	// Remove clients
	for (u32 ii = 0; ii < array::size(to_remove); ++ii)
		console_server_internal::remove_client(*this, to_remove[ii]);
}

void ConsoleServer::register_command_name(const char* name, const char* brief, CommandTypeFunction function, void* user_data)
{
	CE_ENSURE(NULL != name);
	CE_ENSURE(NULL != brief);
	CE_ENSURE(NULL != function);

	CommandData cmd;
	cmd.command_function = function;
	cmd.user_data = user_data;
	strncpy(cmd.name, name, sizeof(cmd.name)-1);
	strncpy(cmd.brief, brief, sizeof(cmd.brief)-1);
	hash_map::set(_commands, StringId32(name), cmd);
}

void ConsoleServer::register_message_type(const char* type, MessageTypeFunction function, void* user_data)
{
	CE_ENSURE(NULL != type);
	CE_ENSURE(NULL != function);

	CommandData cmd;
	cmd.message_function = function;
	cmd.user_data = user_data;
	hash_map::set(_messages, StringId32(type), cmd);
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
