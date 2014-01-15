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
#include "IdArray.h"
#include "Log.h"

#define MAX_CONSOLE_CLIENTS 32

namespace crown
{

struct Client
{
	Id id;
	TCPSocket socket;

	void close()
	{
		socket.close();
	}
};

typedef IdArray<MAX_CONSOLE_CLIENTS, Client> ClientArray;

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

	/// Collects requests from clients and processes them all.
	void						update();

private:

	void						send(TCPSocket client, const char* message);
	void						send_to_all(const char* message);

	void						add_client(TCPSocket socket);
	ReadResult					update_client(TCPSocket client);
	void						process(TCPSocket client, const char* request);

	void						process_ping(TCPSocket client, const char* msg);
	void						process_script(TCPSocket client, const char* msg);
	void						process_stats(TCPSocket client, const char* msg);
	void						process_command(TCPSocket client, const char* msg);
	void						processs_filesystem(TCPSocket client, const char* msg);

private:

	uint16_t					m_port;
	TCPServer					m_server;
	ClientArray					m_clients;
};

} // namespace crown
