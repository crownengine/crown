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

#include "OsSocket.h"
#include "List.h"
#include "Queue.h"
#include "IdTable.h"
#include "Log.h"

namespace crown
{

class RPCHandler;
typedef Id ClientId;
#define MAX_RPC_CLIENTS 16

struct RPCCallback
{
	RPCHandler* handler;
	ClientId client;
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

	void				log_to_all(const char* message, LogSeverity::Enum severity);

	void				send_message_to(ClientId client, const char* message);
	void				send_message_to_all(const char* message);

	void				execute_callbacks();

private:

	void				update_client(ClientId id);
	void				add_client(TCPSocket& client);
	void				remove_client(ClientId id);

	// Returns a handler for processing type messages or NULL.
	RPCHandler*			find_handler(const char* type);
	void				push_callback(RPCHandler* handler, ClientId client, uint32_t message_index);

private:

	TCPListener					m_listener;

	uint8_t						m_num_clients;
	IdTable<MAX_RPC_CLIENTS> 	m_clients_table;
	TCPSocket					m_clients[MAX_RPC_CLIENTS];

	List<char>					m_receive_buffer;
	List<char>					m_send_buffer;

	RPCHandler*					m_handlers_head;

	Queue<RPCCallback>			m_receive_callbacks;
	Queue<RPCCallback>			m_send_callbacks;
};

} // namespace crown
