/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "console_server.h"
#include "json_object.h"
#include "sjson.h"
#include "sort_map.h"
#include "string_id.h"
#include "string_stream.h"
#include "temp_allocator.h"

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
	TempAllocator4096 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"error\",\"message\":\"" << msg << "\"}";
	send(client, string_stream::c_str(ss));
}

void ConsoleServer::success(TCPSocket client, const char* msg)
{
	TempAllocator4096 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"success\",\"message\":\"" << msg << "\"}";
	send(client, string_stream::c_str(ss));
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
	JsonObject obj(ta);
	sjson::parse(json, obj);

	CommandData cd;
	cd.cmd = NULL;
	cd.data = NULL;
	cd = sort_map::get(_commands, sjson::parse_string_id(obj["type"]), cd);

	if (cd.cmd)
		cd.cmd(cd.data, *this, client, json);
	else
		error(client, "Unknown command");
}

void ConsoleServer::register_command(StringId32 type, CommandFunction cmd, void* data)
{
	CE_ASSERT(!sort_map::has(_commands, type), "Command type already registered");
	CE_ASSERT_NOT_NULL(cmd);

	CommandData cd;
	cd.cmd = cmd;
	cd.data = data;

	sort_map::set(_commands, type, cd);
	sort_map::sort(_commands);
}

} // namespace crown
