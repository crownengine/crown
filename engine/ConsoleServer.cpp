/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ConsoleServer.h"
#include "JSONParser.h"
#include "TempAllocator.h"
#include "StringStream.h"
#include "Log.h"
#include "Device.h"
#include "ProxyAllocator.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
ConsoleServer::ConsoleServer()
	: m_num_clients(0)
	, m_receive_buffer(default_allocator())
	, m_send_buffer(default_allocator())
	, m_receive_callbacks(default_allocator())
	, m_send_callbacks(default_allocator())
{
}

//-----------------------------------------------------------------------------
void ConsoleServer::init(bool wait)
{
	m_listener.open(10001);

	if (wait)
	{
		TCPSocket client;
		while (!m_listener.listen(client)) ;

		add_client(client);
	}
}

//-----------------------------------------------------------------------------
void ConsoleServer::shutdown()
{
	for (uint32_t i = 0; i < MAX_CONSOLE_CLIENTS; i++)
	{
		m_clients[i].close();
	}

	m_listener.close();
}

//-----------------------------------------------------------------------------
void ConsoleServer::log_to_all(const char* message, LogSeverity::Enum severity)
{
	TempAllocator2048 alloc;
	StringStream json(alloc);

	static const char* severity_to_text[] = { "info", "warning", "error", "debug" };

	json << "{\"type\":\"message\",";
	json << "\"severity\":\"" << severity_to_text[severity] << "\",";

	char buf[1024];
	string::strncpy(buf, message, 1024);
	for (uint32_t i = 0; i < string::strlen(message); i++)
	{
		if (buf[i] == '"')
			buf[i] = '\'';
	}

	json << "\"message\":\"" << buf << "\"}";

	send_message_to_all(json.c_str());
}

//-----------------------------------------------------------------------------
void ConsoleServer::send_message_to(ClientId client, const char* message)
{
	RPCCallback cb;
	cb.client = client;
	cb.message_index = m_send_buffer.size();

	m_send_buffer.push(message, string::strlen(message));
	m_send_buffer.push_back('\0');

	m_send_callbacks.push_back(cb);
}

//-----------------------------------------------------------------------------
void ConsoleServer::send_message_to_all(const char* message)
{
	// Update all clients
	for (uint32_t i = 0; i < MAX_CONSOLE_CLIENTS; i++)
	{
		ClientId id = *(m_clients_table.begin() + i);
		if (id.id != INVALID_ID)
		{
			send_message_to(id, message);
		}
	}
}

//-----------------------------------------------------------------------------
void ConsoleServer::update()
{
	// Check for new clients
	TCPSocket client;
	if (m_listener.listen(client))
	{
		add_client(client);
	}

	// Update all clients
	for (uint32_t i = 0; i < MAX_CONSOLE_CLIENTS; i++)
	{
		ClientId id = *(m_clients_table.begin() + i);
		if (id.id != INVALID_ID)
		{
			update_client(id);
		}
	}
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_requests()
{
	for (uint32_t i = 0; i < m_receive_callbacks.size(); i++)
	{
		RPCCallback cb = m_receive_callbacks.front();
		m_receive_callbacks.pop_front();

		const char* request = &m_receive_buffer[cb.message_index];
		JSONParser parser(request);
		JSONElement request_type = parser.root().key("type");
		const char* type = request_type.string_value();

		// Determine request type
		if (string::strcmp("ping", type) == 0) process_ping(cb.client, request);
		else if (string::strcmp("script", type) == 0) process_script(cb.client, request);
		else if (string::strcmp("stats", type) == 0) process_stats(cb.client, request);
		else if (string::strcmp("command", type) == 0) process_command(cb.client, request);
		else continue;
	}

	m_receive_callbacks.clear();
	m_receive_buffer.clear();

	for (uint32_t i = 0; i < m_send_callbacks.size(); i++)
	{
		RPCCallback cb = m_send_callbacks.front();
	 	m_send_callbacks.pop_front();

	 	m_clients[cb.client.index].write(&m_send_buffer[cb.message_index], string::strlen(&m_send_buffer[cb.message_index]));
	}

	m_send_callbacks.clear();
	m_send_buffer.clear();
}

//-----------------------------------------------------------------------------
void ConsoleServer::update_client(ClientId id)
{
	size_t total_read = 0;
	uint32_t message_index = m_receive_buffer.size();

	TCPSocket& client = m_clients[id.index];

	char buf[1024];
	while (true)
	{
		ReadResult result = client.read(buf, 32);

		if (result.error == ReadResult::NO_RESULT_ERROR)
		{
			if (result.received_bytes > 0)
			{
				m_receive_buffer.push(buf, result.received_bytes);
				total_read += result.received_bytes;
				continue;
			}

			break;
		}
		else
		{
			// Close remote connection
			client.close();
			remove_client(id);
			return;
		}
	}

	// Ensure NUL-terminated
	m_receive_buffer.push_back('\0');

	// Process only if received something
	if (total_read > 0)
	{
		add_request(id, message_index);
	}
}

//-----------------------------------------------------------------------------
void ConsoleServer::add_client(TCPSocket& client)
{
	if (m_num_clients < MAX_CONSOLE_CLIENTS)
	{
		ClientId id = m_clients_table.create();
		m_clients[id.index] = client;
		m_num_clients++;
	}
	else
	{
		Log::e("Too many clients, connection denied");
	}
}

//-----------------------------------------------------------------------------
void ConsoleServer::remove_client(ClientId id)
{
	CE_ASSERT(m_num_clients > 0, "No client connected");

	m_clients[id.index].close();
	m_clients_table.destroy(id);
	m_num_clients--;
}

//-----------------------------------------------------------------------------
void ConsoleServer::add_request(ClientId client, uint32_t message_index)
{
	RPCCallback cb;
	cb.client = client;
	cb.message_index = message_index;
	m_receive_callbacks.push_back(cb);
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_ping(ClientId client, const char* /*msg*/)
{
	send_message_to(client, "{\"type\":\"pong\"}");
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_script(ClientId /*client*/, const char* msg)
{
	JSONParser parser(msg);
	JSONElement root = parser.root();

	const char* script = root.key("script").string_value();
	device()->lua_environment()->execute_string(script);
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_stats(ClientId client, const char* /*msg*/)
{
	TempAllocator2048 alloc;
	StringStream response(alloc);

	response << "{\"type\":\"response\",";
	response << "{\"allocators\":[";

	// Walk all proxy allocators
	ProxyAllocator* proxy = ProxyAllocator::begin();
	while (proxy != NULL)
	{
		response << "{";
		response << "\"name\":\"" << proxy->name() << "\",";
		response << "\"allocated_size\":\"" << proxy->allocated_size() << "\"";
		response << "},";

		proxy = ProxyAllocator::next(proxy);
	}

	response << "]" << "}";

	send_message_to(client, response.c_str());
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_command(ClientId /*client*/, const char* msg)
{
	JSONParser parser(msg);
	JSONElement root = parser.root();
	JSONElement command = root.key("command");

	const char* cmd = command.string_value();

	if (string::strcmp("reload", cmd) == 0)
	{
		JSONElement resource_type = root.key_or_nil("resource_type");
		JSONElement resource_name = root.key_or_nil("resource_name");

		char t[256];
		char n[256];
		string::strncpy(t, resource_type.string_value(), 256);
		string::strncpy(n, resource_name.string_value(), 256);
		device()->reload(t, n);
	}
	else if (string::strcmp("pause", cmd) == 0)
	{
		device()->pause();
	}
	else if (string::strcmp("unpause", cmd) == 0)
	{
		device()->unpause();
	}
}

} // namespace crown
