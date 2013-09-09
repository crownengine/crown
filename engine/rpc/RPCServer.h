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

#pragma once

#include "Log.h"
#include "TCPSocket.h"
#include "List.h"
#include "Queue.h"
#include "JSONParser.h"
#include "RPCHandler.h"

namespace crown
{

struct RPCCallback
{
	RPCHandler* handler;
	TCPClient* client;
	uint32_t message_index;
};

class RPCServer
{
public:

						RPCServer();

	void				add_handler(RPCHandler* handler);
	void				remove_handler(RPCHandler* handler);

	void				init(bool wait);
	void				shutdown();

	void				update();

	void				send_message_to(TCPClient* client, const char* message);

	void				execute_callbacks();

private:

	void				update_client(TCPClient& client);
	void				add_client(TCPClient& client);

	// Returns a handler for processing type messages or NULL.
	RPCHandler*			find_handler(const char* type);
	void				push_callback(RPCHandler* handler, TCPClient* client, uint32_t message_index);

private:

	TCPListener			m_listener;
	List<TCPClient>		m_clients;

	List<char>			m_receive_buffer;
	List<char>			m_send_buffer;

	RPCHandler*			m_handlers_head;

	Queue<RPCCallback>	m_receive_callbacks;
	Queue<RPCCallback>	m_send_callbacks;
};

//-----------------------------------------------------------------------------
RPCServer::RPCServer()
	: m_listener(10001), m_clients(default_allocator()), m_receive_buffer(default_allocator()),
		m_send_buffer(default_allocator()), m_handlers_head(NULL), m_receive_callbacks(default_allocator()),
		m_send_callbacks(default_allocator())
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
	if (wait)
	{
		TCPClient client;
		while (!m_listener.listen(client) && wait) ;
		add_client(client);
	}
}

//-----------------------------------------------------------------------------
void RPCServer::shutdown()
{
	for (uint32_t i = 0; i < m_clients.size(); i++)
	{
		m_clients[i].close();
	}

	m_listener.close();
}

//-----------------------------------------------------------------------------
void RPCServer::update()
{
	TCPClient client;
	if (m_listener.listen(client))
	{
		add_client(client);
	}

	for (uint32_t i = 0; i < m_clients.size(); i++)
	{
		update_client(m_clients[i]);
	}
}

//-----------------------------------------------------------------------------
void RPCServer::send_message_to(TCPClient* client, const char* message)
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

		cb.client->write(&m_send_buffer[cb.message_index], string::strlen(&m_send_buffer[cb.message_index]));
	}

	m_send_callbacks.clear();
	m_send_buffer.clear();
}

//-----------------------------------------------------------------------------
void RPCServer::update_client(TCPClient& client)
{
	size_t total_read = 0;
	size_t bytes_read = 0;
	uint32_t message_index = m_receive_buffer.size();

	char buf[1024];

	while ((bytes_read = client.read(buf, 10)) > 0)
	{
		m_receive_buffer.push(buf, bytes_read);
		total_read += bytes_read;
	}

	m_receive_buffer.push_back('\0');

	if (total_read > 0)
	{
		JSONParser parser(&m_receive_buffer[message_index]);
		JSONElement root = parser.root();

		const char* type_name = root.key("type").string_value();
		uint32_t type_id = hash::murmur2_32(type_name, strlen(type_name), 0);

		Log::i("Message: %s", &m_receive_buffer[message_index]);
		Log::i("Type: %s (%.8X)", type_name, type_id);

		RPCHandler* handler = find_handler(type_name);
		if (handler != NULL)
		{
			push_callback(handler, &client, message_index);
		}
	}
}

//-----------------------------------------------------------------------------
void RPCServer::add_client(TCPClient& client)
{
	m_clients.push_back(client);
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
void RPCServer::push_callback(RPCHandler* handler, TCPClient* client, uint32_t message_index)
{
	RPCCallback cb;
	cb.handler = handler;
	cb.client = client;
	cb.message_index = message_index;

	m_receive_callbacks.push_back(cb);
}

} // namespace crown
