/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "console_server.h"
#include "temp_allocator.h"
#include "string_stream.h"
#include "device.h"
#include "lua_environment.h"
#include "memory.h"
#include "dynamic_string.h"
#include "json.h"
#include "map.h"

namespace crown
{

ConsoleServer::ConsoleServer(uint16_t port, bool wait)
{
	m_server.bind(port);
	m_server.listen(5);

	if (wait)
	{
		AcceptResult result;
		TCPSocket client;
		do
		{
			result = m_server.accept(client);
		}
		while (result.error != AcceptResult::NO_ERROR);

		add_client(client);
	}
}

void ConsoleServer::shutdown()
{
	for (uint32_t i = 0; i < id_array::size(m_clients); i++)
	{
		m_clients[i].close();
	}

	m_server.close();
}

namespace console_server_internal
{
	StringStream& sanitize(StringStream& ss, const char* msg)
	{
		using namespace string_stream;
		const char* ch = msg;
		for (; *ch; ch++)
		{
			if (*ch == '"')
				ss << "\\";
			ss << *ch;
		}

		return ss;
	}
}

void ConsoleServer::log(const char* msg, LogSeverity::Enum severity)
{
	using namespace string_stream;
	using namespace console_server_internal;
	static const char* stt[] = { "info", "warning", "error", "debug" };

	// Build json message
	TempAllocator2048 alloc;
	StringStream json(alloc);

	json << "{\"type\":\"message\",";
	json << "\"severity\":\"" << stt[severity] << "\",";
	json << "\"message\":\""; sanitize(json, msg) << "\"}";

	send(c_str(json));
}

void ConsoleServer::send(TCPSocket client, const char* json)
{
	uint32_t len = strlen(json);
	client.write((const char*)&len, 4);
	client.write(json, len);
}

void ConsoleServer::send(const char* json)
{
	for (uint32_t i = 0; i < id_array::size(m_clients); i++)
	{
		send(m_clients[i].socket, json);
	}
}

void ConsoleServer::update()
{
	// Check for new clients only if we have room for them
	if (id_array::size(m_clients) < CE_MAX_CONSOLE_CLIENTS - 1)
	{
		TCPSocket client;
		AcceptResult result = m_server.accept_nonblock(client);
		if (result.error == AcceptResult::NO_ERROR)
		{
			add_client(client);
		}
	}

	TempAllocator256 alloc;
	Array<Id> to_remove(alloc);

	// Update all clients
	for (uint32_t i = 0; i < id_array::size(m_clients); i++)
	{
		ReadResult rr = update_client(m_clients[i].socket);
		if (rr.error != ReadResult::NO_ERROR) array::push_back(to_remove, m_clients[i].id);
	}

	// Remove clients
	for (uint32_t i = 0; i < array::size(to_remove); i++)
	{
		id_array::get(m_clients, to_remove[i]).socket.close();
		id_array::destroy(m_clients, to_remove[i]);
	}
}

void ConsoleServer::add_client(TCPSocket socket)
{
	Client client;
	client.socket = socket;
	Id id = id_array::create(m_clients, client);
	id_array::get(m_clients, id).id = id;
}

ReadResult ConsoleServer::update_client(TCPSocket client)
{
	uint32_t msg_len = 0;
	ReadResult rr = client.read_nonblock(&msg_len, 4);

	// If no data received, return
	if (rr.error == ReadResult::NO_ERROR && rr.bytes_read == 0) return rr;
	if (rr.error == ReadResult::REMOTE_CLOSED) return rr;
	if (rr.error != ReadResult::NO_ERROR) return rr;

	// Else read the message
	Array<char> msg_buf(default_allocator());
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
	Map<DynamicString, const char*> root(default_allocator());
	json::parse_object(json, root);

	DynamicString type;
	json::parse_string(root["type"], type);

	if (type == "ping") process_ping(client, json);
	else if (type == "script") process_script(client, json);
	else if (type == "command") process_command(client, json);
	else CE_FATAL("Request unknown.");
}

void ConsoleServer::process_ping(TCPSocket client, const char* /*json*/)
{
	send(client, "{\"type\":\"pong\"}");
}

void ConsoleServer::process_script(TCPSocket /*client*/, const char* json)
{
	Map<DynamicString, const char*> root(default_allocator());
	json::parse_object(json, root);

	DynamicString script;
	json::parse_string(root["script"], script);
	device()->lua_environment()->execute_string(script.c_str());
}

void ConsoleServer::process_command(TCPSocket /*client*/, const char* json)
{
	Map<DynamicString, const char*> root(default_allocator());
	json::parse_object(json, root);

	DynamicString cmd;
	json::parse_string(root["command"], cmd);

	if (cmd == "reload")
	{
		DynamicString type;
		DynamicString name;
		json::parse_string(root["resource_type"], type);
		json::parse_string(root["resource_name"], name);

		device()->reload(type.c_str(), name.c_str());
	}
	else if (cmd == "pause")
	{
		device()->pause();
	}
	else if (cmd == "unpause")
	{
		device()->unpause();
	}
}

namespace console_server_globals
{
	char _buffer[sizeof(ConsoleServer)];
	ConsoleServer* _console = NULL;

	void init(uint16_t port, bool wait)
	{
		_console = new (_buffer) ConsoleServer(port, wait);
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

	ConsoleServer& console()
	{
		return *_console;
	}
} // namespace console_server
} // namespace crown
