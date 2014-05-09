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
#include "ContainerTypes.h"
#include "Queue.h"
#include "IdArray.h"
#include "Config.h"

namespace crown
{

/// Enumerates log levels.
struct LogSeverity
{
	enum Enum
	{
		INFO	= 0,
		WARN	= 1,
		ERROR	= 2,
		DEBUG	= 3
	};	
};

struct Client
{
	Id id;
	TCPSocket socket;

	void close()
	{
		socket.close();
	}
};

typedef IdArray<CE_MAX_CONSOLE_CLIENTS, Client> ClientArray;

class ConsoleServer
{
public:

	/// Listens on the given @a port. If @a wait is true, this function
	/// blocks until a client is connected.
	void init(uint16_t port, bool wait);
	void shutdown();

	void log_to_all(LogSeverity::Enum severity, const char* message, ...);
	void log_to_all(LogSeverity::Enum severity, const char* message, ::va_list arg);

	/// Collects requests from clients and processes them all.
	void update();

private:

	void send(TCPSocket client, const char* message);
	void send_to_all(const char* message);

	void add_client(TCPSocket socket);
	ReadResult update_client(TCPSocket client);
	void process(TCPSocket client, const char* request);

	void process_ping(TCPSocket client, const char* msg);
	void process_script(TCPSocket client, const char* msg);
	void process_stats(TCPSocket client, const char* msg);
	void process_command(TCPSocket client, const char* msg);
	void processs_filesystem(TCPSocket client, const char* msg);

private:

	TCPServer m_server;
	ClientArray m_clients;
};

} // namespace crown
