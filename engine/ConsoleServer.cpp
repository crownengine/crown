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
#include "File.h"
#include "Filesystem.h"
#include "MathUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
ConsoleServer::ConsoleServer(uint16_t port)
	: m_port(port)
	, m_num_clients(0)
	, m_receive_buffer(default_allocator())
	, m_receive_callbacks(default_allocator())
{
}

//-----------------------------------------------------------------------------
void ConsoleServer::init(bool wait)
{
	m_server.open(m_port);
	m_server.listen(5);

	if (wait)
	{
		TCPSocket client;
		m_server.accept(client);
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

	m_server.close();
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
	uint32_t msg_len = string::strlen(message);
	m_clients[client.index].write((const char*) &msg_len, 4);
	m_clients[client.index].write(message, msg_len);
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
	if (m_server.accept_nonblock(client))
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

	// Process all requests
	process_requests();
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
		DynamicString type; 
		request_type.string_value(type);

		// Determine request type
		if (type == "ping") process_ping(cb.client, request);
		else if (type == "script") process_script(cb.client, request);
		else if (type == "stats") process_stats(cb.client, request);
		else if (type == "command") process_command(cb.client, request);
		else if (type == "filesystem") processs_filesystem(cb.client, request);
		else continue;
	}

	m_receive_callbacks.clear();
	m_receive_buffer.clear();
}

//-----------------------------------------------------------------------------
void ConsoleServer::update_client(ClientId id)
{
	TCPSocket& client = m_clients[id.index];

	uint32_t msg_len = 0;
	ReadResult rr = client.read_nonblock(&msg_len, 4);

	// If no data received, return
	if (rr.error == ReadResult::NO_ERROR && rr.received_bytes == 0) return;
	if (rr.error == ReadResult::REMOTE_CLOSED)
	{
		remove_client(id);
		return;
	}

	// Else read the message
	List<char> msg_buf(default_allocator());
	msg_buf.resize(msg_len);
	ReadResult msg_result = client.read(msg_buf.begin(), msg_len);

	uint32_t message_index = m_receive_buffer.size();
	m_receive_buffer.push(msg_buf.begin(), msg_result.received_bytes);
	m_receive_buffer.push_back('\0');
	add_request(id, message_index);
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

	DynamicString script;
	root.key("script").string_value(script);
	device()->lua_environment()->execute_string(script.c_str());
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

	DynamicString cmd;
	command.string_value(cmd);

	if (cmd == "reload")
	{
		JSONElement type = root.key_or_nil("resource_type");
		JSONElement name = root.key_or_nil("resource_name");

		DynamicString resource_type;
		DynamicString resource_name;
		type.string_value(resource_type);
		name.string_value(resource_name);

		char t[256];
		char n[256];
		string::strncpy(t, resource_type.c_str(), 256);
		string::strncpy(n, resource_name.c_str(), 256);
		device()->reload(t, n);
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

//-----------------------------------------------------------------------------
void ConsoleServer::processs_filesystem(ClientId client, const char* msg)
{
	JSONParser parser(msg);
	JSONElement root = parser.root();
	JSONElement filesystem = root.key("filesystem");

	DynamicString cmd;
	filesystem.string_value(cmd);

	if (cmd == "size")
	{
		DynamicString file_name;
		root.key("file").string_value(file_name);

		File* file = device()->filesystem()->open(file_name.c_str(), FOM_READ);
		size_t file_size = file->size();
		device()->filesystem()->close(file);

		TempAllocator64 alloc;
		StringStream response(alloc);
		response << "{\"type\":\"file\",\"size\":" << file_size << "}";

		send_message_to(client, response.c_str());
	}
	else if (cmd == "read")
	{
		JSONElement file_position = root.key("position");
		JSONElement file_size = root.key("size");

		DynamicString file_name;
		root.key("file").string_value(file_name);

		// Read the file data
		File* file = device()->filesystem()->open(file_name.c_str(), FOM_READ);
		char* bytes = (char*) default_allocator().allocate((size_t) file_size.int_value());
		file->seek((size_t) file_position.int_value());
		file->read(bytes, (size_t) file_size.int_value());
		device()->filesystem()->close(file);

		// Encode data to base64
		size_t dummy;
		char* bytes_encoded = math::base64_encode((unsigned char*) bytes, (size_t) file_size.int_value(), &dummy);

		// Send data
		TempAllocator4096 alloc;
		StringStream response(alloc);

		response << "{\"type\":\"file\",";
		response << "\"data\":\"" << bytes_encoded << "\"}";

		send_message_to(client, response.c_str());

		// Cleanup
		default_allocator().deallocate(bytes_encoded);
		default_allocator().deallocate(bytes);
	}
}

} // namespace crown
