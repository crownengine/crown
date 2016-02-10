/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "bundle_compiler.h"
#include "console_server.h"
#include "device.h"
#include "dynamic_string.h"
#include "lua_environment.h"
#include "map.h"
#include "memory.h"
#include "sjson.h"
#include "string_stream.h"
#include "temp_allocator.h"

namespace crown
{
ConsoleServer::ConsoleServer(Allocator& a)
	: _clients(a)
{
}

void ConsoleServer::init(u16 port, bool wait)
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
		while (ar.error != AcceptResult::NO_ERROR);

		add_client(client);
	}
}

void ConsoleServer::shutdown()
{
	for (u32 i = 0; i < vector::size(_clients); ++i)
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
	using namespace string_stream;
	TempAllocator4096 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"error\",\"message\":\"" << msg << "\"}";
	send(client, c_str(ss));
}

void ConsoleServer::success(TCPSocket client, const char* msg)
{
	using namespace string_stream;
	TempAllocator4096 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"success\",\"message\":\"" << msg << "\"}";
	send(client, c_str(ss));
}

void ConsoleServer::send(const char* json)
{
	for (u32 i = 0; i < vector::size(_clients); ++i)
		send(_clients[i], json);
}

void ConsoleServer::update()
{
	TCPSocket client;
	AcceptResult result = _server.accept_nonblock(client);
	if (result.error == AcceptResult::NO_ERROR)
		add_client(client);

	TempAllocator256 alloc;
	Array<u32> to_remove(alloc);

	// Update all clients
	for (u32 i = 0; i < vector::size(_clients); ++i)
	{
		ReadResult rr = update_client(_clients[i]);
		if (rr.error != ReadResult::NO_ERROR)
			array::push_back(to_remove, i);
	}

	// Remove clients
	for (u32 i = 0; i < array::size(to_remove); ++i)
	{
		const u32 last = vector::size(_clients) - 1;
		const u32 c = to_remove[i];

		_clients[c].close();
		_clients[c] = _clients[last];
		vector::pop_back(_clients);
	}
}

void ConsoleServer::add_client(TCPSocket socket)
{
	vector::push_back(_clients, socket);
}

ReadResult ConsoleServer::update_client(TCPSocket client)
{
	u32 msg_len = 0;
	ReadResult rr = client.read_nonblock(&msg_len, 4);

	// If no data received, return
	if (rr.error == ReadResult::NO_ERROR && rr.bytes_read == 0) return rr;
	if (rr.error == ReadResult::REMOTE_CLOSED) return rr;
	if (rr.error != ReadResult::NO_ERROR) return rr;

	// Else read the message
	TempAllocator4096 ta;
	Array<char> msg_buf(ta);
	array::resize(msg_buf, msg_len);
	ReadResult msg_result = client.read(array::begin(msg_buf), msg_len);
	array::push_back(msg_buf, '\0');

	if (msg_result.error == ReadResult::REMOTE_CLOSED) return msg_result;
	if (msg_result.error != ReadResult::NO_ERROR) return msg_result;

	process(client, array::begin(msg_buf));
	return msg_result;
}

void ConsoleServer::process(TCPSocket client, const char* json)
{
	TempAllocator4096 ta;
	JsonObject root(ta);
	sjson::parse(json, root);

	DynamicString type(ta);
	sjson::parse_string(root["type"], type);

	if (type == "script")
	{
		DynamicString script(ta);
		sjson::parse_string(root["script"], script);

		device()->lua_environment()->execute_string(script.c_str());

		success(client, "Script executed.");
	}
	else if (type == "compile")
	{
		DynamicString rtype(ta);
		DynamicString rname(ta);
		DynamicString platform(ta);
		sjson::parse_string(root["resource_type"], rtype);
		sjson::parse_string(root["resource_name"], rname);
		sjson::parse_string(root["platform"], platform);

		bool succ = bundle_compiler_globals::compiler()->compile(rtype.c_str()
			, rname.c_str()
			, platform.c_str()
			);

		if (succ)
			success(client, "Resource compiled.");
		else
			error(client, "Failed to compile resource.");
	}
	else if (type == "reload")
	{
		StringId64 rtype = sjson::parse_resource_id(root["resource_type"]);
		StringId64 rname = sjson::parse_resource_id(root["resource_name"]);

		device()->reload(rtype, rname);
	}
	else if (type == "pause")
	{
		device()->pause();
	}
	else if (type == "unpause")
	{
		device()->unpause();
	}
	else
	{
		error(client, "Unknown command");
	}
}

namespace console_server_globals
{
	char _buffer[sizeof(ConsoleServer)];
	ConsoleServer* _console = NULL;

	void init(u16 port, bool wait)
	{
		_console = new (_buffer) ConsoleServer(default_allocator());
		_console->init(port, wait);
	}

	void shutdown()
	{
		_console->~ConsoleServer();
		_console = NULL;
	}

	void update()
	{
#if CROWN_DEBUG
		_console->update();
#endif // CROWN_DEBUG
	}

	ConsoleServer* console()
	{
		return _console;
	}
} // namespace console_server
} // namespace crown
