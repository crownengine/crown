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
{
}

//-----------------------------------------------------------------------------
void ConsoleServer::init(bool wait)
{
	m_server.open(m_port);
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

//-----------------------------------------------------------------------------
void ConsoleServer::shutdown()
{
	for (uint32_t i = 0; i < m_clients.size(); i++)
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

	send_to_all(json.c_str());
}

//-----------------------------------------------------------------------------
void ConsoleServer::send(TCPSocket client, const char* message)
{
	uint32_t msg_len = string::strlen(message);
	client.write((const char*) &msg_len, 4);
	client.write(message, msg_len);
}

//-----------------------------------------------------------------------------
void ConsoleServer::send_to_all(const char* message)
{
	for (uint32_t i = 0; i < m_clients.size(); i++)
	{
		send(m_clients[i].socket, message);
	}
}

//-----------------------------------------------------------------------------
void ConsoleServer::update()
{
	// Check for new clients only if we have room for them
	if (m_clients.size() < CE_MAX_CONSOLE_CLIENTS - 1)
	{
		TCPSocket client;
		AcceptResult result = m_server.accept_nonblock(client);
		if (result.error == AcceptResult::NO_ERROR)
		{
			add_client(client);
		}
	}

	TempAllocator256 alloc;
	List<Id> to_remove(alloc);

	// Update all clients
	for (uint32_t i = 0; i < m_clients.size(); i++)
	{
		ReadResult rr = update_client(m_clients[i].socket);
		if (rr.error != ReadResult::NO_ERROR) to_remove.push_back(m_clients[i].id);
	}

	// Remove clients
	for (uint32_t i = 0; i < to_remove.size(); i++)
	{
		m_clients.lookup(to_remove[i]).socket.close();
		m_clients.destroy(to_remove[i]);
	}
}

//-----------------------------------------------------------------------------
void ConsoleServer::add_client(TCPSocket socket)
{
	Client client;
	client.socket = socket;
	Id id = m_clients.create(client);
	m_clients.lookup(id).id = id;
}

//-----------------------------------------------------------------------------
ReadResult ConsoleServer::update_client(TCPSocket client)
{
	uint32_t msg_len = 0;
	ReadResult rr = client.read_nonblock(&msg_len, 4);

	// If no data received, return
	if (rr.error == ReadResult::NO_ERROR && rr.bytes_read == 0) return rr;
	if (rr.error == ReadResult::REMOTE_CLOSED) return rr;
	if (rr.error != ReadResult::NO_ERROR) return rr;

	// Else read the message
	List<char> msg_buf(default_allocator());
	msg_buf.resize(msg_len);
	ReadResult msg_result = client.read(msg_buf.begin(), msg_len);
	msg_buf.push_back('\0');

	if (msg_result.error == ReadResult::REMOTE_CLOSED) return msg_result;
	if (msg_result.error != ReadResult::NO_ERROR) return msg_result;

	process(client, msg_buf.begin());
	return msg_result;
}

//-----------------------------------------------------------------------------
void ConsoleServer::process(TCPSocket client, const char* request)
{
	JSONParser parser(request);
	DynamicString type; 
	parser.root().key("type").to_string(type);

	// Determine request type
	if (type == "ping") process_ping(client, request);
	else if (type == "script") process_script(client, request);
	else if (type == "stats") process_stats(client, request);
	else if (type == "command") process_command(client, request);
	else if (type == "filesystem") processs_filesystem(client, request);
	else CE_FATAL("Request unknown.");
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_ping(TCPSocket client, const char* /*msg*/)
{
	send(client, "{\"type\":\"pong\"}");
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_script(TCPSocket /*client*/, const char* msg)
{
	JSONParser parser(msg);
	JSONElement root = parser.root();

	DynamicString script;
	root.key("script").to_string(script);
	device()->lua_environment()->execute_string(script.c_str());
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_stats(TCPSocket client, const char* /*msg*/)
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

	send(client, response.c_str());
}

//-----------------------------------------------------------------------------
void ConsoleServer::process_command(TCPSocket /*client*/, const char* msg)
{
	JSONParser parser(msg);
	JSONElement root = parser.root();
	JSONElement command = root.key("command");

	DynamicString cmd;
	command.to_string(cmd);

	if (cmd == "reload")
	{
		JSONElement type = root.key_or_nil("resource_type");
		JSONElement name = root.key_or_nil("resource_name");

		DynamicString resource_type;
		DynamicString resource_name;
		type.to_string(resource_type);
		name.to_string(resource_name);

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
void ConsoleServer::processs_filesystem(TCPSocket client, const char* msg)
{
	JSONParser parser(msg);
	JSONElement root = parser.root();
	JSONElement filesystem = root.key("filesystem");

	DynamicString cmd;
	filesystem.to_string(cmd);

	if (cmd == "size")
	{
		DynamicString file_name;
		root.key("file").to_string(file_name);

		File* file = device()->filesystem()->open(file_name.c_str(), FOM_READ);
		size_t file_size = file->size();
		device()->filesystem()->close(file);

		TempAllocator64 alloc;
		StringStream response(alloc);
		response << "{\"type\":\"file\",\"size\":" << file_size << "}";

		send(client, response.c_str());
	}
	else if (cmd == "read")
	{
		JSONElement file_position = root.key("position");
		JSONElement file_size = root.key("size");

		DynamicString file_name;
		root.key("file").to_string(file_name);

		// Read the file data
		File* file = device()->filesystem()->open(file_name.c_str(), FOM_READ);
		char* bytes = (char*) default_allocator().allocate((size_t) file_size.to_int());
		file->seek((size_t) file_position.to_int());
		file->read(bytes, (size_t) file_size.to_int());
		device()->filesystem()->close(file);

		// Encode data to base64
		size_t dummy;
		char* bytes_encoded = math::base64_encode((unsigned char*) bytes, (size_t) file_size.to_int(), &dummy);

		// Send data
		TempAllocator4096 alloc;
		StringStream response(alloc);

		response << "{\"type\":\"file\",";
		response << "\"data\":\"" << bytes_encoded << "\"}";

		send(client, response.c_str());

		// Cleanup
		default_allocator().deallocate(bytes_encoded);
		default_allocator().deallocate(bytes);
	}
}

} // namespace crown
