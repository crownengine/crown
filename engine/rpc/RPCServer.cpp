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

#include "RPCServer.h"
#include "Log.h"
#include "JSONParser.h"
#include "RPCHandler.h"
#include "TempAllocator.h"
#include "StringStream.h"

namespace crown
{

//-----------------------------------------------------------------------------
RPCServer::RPCServer()
	: m_num_clients(0)
	, m_receive_buffer(default_allocator())
	, m_send_buffer(default_allocator())
	, m_handlers_head(NULL)
	, m_receive_callbacks(default_allocator())
	, m_send_callbacks(default_allocator())
{
}

//-----------------------------------------------------------------------------
void RPCServer::add_handler(RPCHandler* handler)
{
	CE_ASSERT_NOT_NULL(handler);

	if (m_handlers_head != NULL)
	{
		handler->m_next = m_handlers_head;
	}

	m_handlers_head = handler;
}

//-----------------------------------------------------------------------------
void RPCServer::remove_handler(RPCHandler* handler)
{
	CE_ASSERT_NOT_NULL(handler);

	RPCHandler* cur = m_handlers_head;
	RPCHandler* prev = NULL;

	for (; cur != NULL && cur != handler; prev = cur, cur = cur->m_next) ;

	if (cur == m_handlers_head)
	{
		m_handlers_head = cur->m_next;
	}
	else
	{
		prev->m_next = cur->m_next;
	}
}

//-----------------------------------------------------------------------------
void RPCServer::init(bool wait)
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
void RPCServer::shutdown()
{
	for (uint32_t i = 0; i < MAX_RPC_CLIENTS; i++)
	{
		m_clients[i].close();
	}

	m_listener.close();
}

//-----------------------------------------------------------------------------
void RPCServer::update()
{
	// Check for new clients
	TCPSocket client;
	if (m_listener.listen(client))
	{
		add_client(client);
	}

	// Update all clients
	for (uint32_t i = 0; i < MAX_RPC_CLIENTS; i++)
	{
		ClientId id = *(m_clients_table.begin() + i);
		if (id.id != INVALID_ID)
		{
			update_client(id);
		}
	}
}

//-----------------------------------------------------------------------------
void RPCServer::log_to_all(const char* message, LogSeverity::Enum severity)
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
void RPCServer::send_message_to(ClientId client, const char* message)
{
	RPCCallback cb;
	cb.handler = NULL;
	cb.client = client;
	cb.message_index = m_send_buffer.size();

	m_send_buffer.push(message, string::strlen(message));
	m_send_buffer.push_back('\0');

	m_send_callbacks.push_back(cb);
}

//-----------------------------------------------------------------------------
void RPCServer::send_message_to_all(const char* message)
{
	// Update all clients
	for (uint32_t i = 0; i < MAX_RPC_CLIENTS; i++)
	{
		ClientId id = *(m_clients_table.begin() + i);
		if (id.id != INVALID_ID)
		{
			send_message_to(id, message);
		}
	}
}

//-----------------------------------------------------------------------------
void RPCServer::execute_callbacks()
{
	for (uint32_t i = 0; i < m_receive_callbacks.size(); i++)
	{
		RPCCallback cb = m_receive_callbacks.front();
		m_receive_callbacks.pop_front();

		cb.handler->execute_command(this, cb.client, &m_receive_buffer[cb.message_index]);
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
void RPCServer::update_client(ClientId id)
{
	size_t total_read = 0;
	uint32_t message_index = m_receive_buffer.size();

	TCPSocket& client = m_clients[id.index];

	char buf[1024];
	while (true)
	{
		ReadResult result = client.read(buf, 32);

		if (result.error == ReadResult::NO_ERROR)
		{
			if (result.received_bytes > 0)
			{
				for (uint32_t i = 0; i < result.received_bytes; i++)
			 	{
			 		printf("%c", buf[i]);
			 	}
			 	printf("\n");

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
	// TODO: Bad JSON strings crash
	if (total_read > 0)
	{
		JSONParser parser(&m_receive_buffer[message_index]);
		Log::d("%s", &m_receive_buffer[message_index]);
		JSONElement root = parser.root();
		JSONElement type = root.key_or_nil("type");

		if (!type.is_nil())
		{
			RPCHandler* handler = find_handler(type.string_value());
			if (handler != NULL)
			{
				push_callback(handler, id, message_index);
			}
		}
		else
		{
			Log::d("Wrong message, type is nil");
		}
	}
}

//-----------------------------------------------------------------------------
void RPCServer::add_client(TCPSocket& client)
{
	if (m_num_clients < MAX_RPC_CLIENTS)
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
void RPCServer::remove_client(ClientId id)
{
	CE_ASSERT(m_num_clients > 0, "No client connected");

	m_clients[id.index].close();
	m_clients_table.destroy(id);
	m_num_clients--;
}

//-----------------------------------------------------------------------------
RPCHandler* RPCServer::find_handler(const char* type)
{
	const RPCHandler* handler = m_handlers_head;

	while (handler != NULL)
	{
		if (handler->type() == hash::murmur2_32(type, string::strlen(type), 0))
		{
			return (RPCHandler*)handler;
		}

		handler = handler->m_next;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void RPCServer::push_callback(RPCHandler* handler, ClientId client, uint32_t message_index)
{
	RPCCallback cb;
	cb.handler = handler;
	cb.client = client;
	cb.message_index = message_index;

	m_receive_callbacks.push_back(cb);
}

} // namespace crown
