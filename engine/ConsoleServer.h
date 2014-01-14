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

typedef Id ClientId;
#define MAX_CONSOLE_CLIENTS 100

struct RPCCallback
{
	ClientId client;
	uint32_t message_index;
};

class ConsoleServer
{
public:

	/// Listens on the given @a port.
								ConsoleServer(uint16_t port);

	/// Initializes the system. If @a wait is true, this function
	/// blocks until a client is connected.
	void						init(bool wait);
	void						shutdown();

	void						log_to_all(const char* message, LogSeverity::Enum severity);

	void						send_message_to(ClientId client, const char* message);
	void						send_message_to_all(const char* message);

	/// Collects requests from clients and processes them all.
	void						update();

private:

	void						process_requests();
	void						update_client(ClientId id);
	void						add_client(TCPSocket& client);
	void						remove_client(ClientId id);

	void						add_request(ClientId client, uint32_t message_index);
	void						process_ping(ClientId client, const char* msg);
	void						process_script(ClientId client, const char* msg);
	void						process_stats(ClientId client, const char* msg);
	void						process_command(ClientId client, const char* msg);
	void						processs_filesystem(ClientId client, const char* msg);

private:

	uint16_t					m_port;
	TCPServer					m_server;

	uint8_t						m_num_clients;
	IdTable<MAX_CONSOLE_CLIENTS>	m_clients_table;
	TCPSocket					m_clients[MAX_CONSOLE_CLIENTS];

	List<char>					m_receive_buffer;
	Queue<RPCCallback>			m_receive_callbacks;
};

} // namespace crown
