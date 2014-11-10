/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "socket.h"
#include "container_types.h"
#include "queue.h"
#include "id_array.h"
#include <cstdarg>

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
	ConsoleServer(uint16_t port, bool wait);
	void shutdown();

	void log_to_all(const char* msg, LogSeverity::Enum severity = LogSeverity::INFO);

	/// Collects requests from clients and processes them all.
	void update();

	/// Sends the given JSON-encoded string to all clients.
	void send_to_all(const char* json);

private:

	void send(TCPSocket client, const char* message);

	void add_client(TCPSocket socket);
	ReadResult update_client(TCPSocket client);
	void process(TCPSocket client, const char* request);

	void process_ping(TCPSocket client, const char* msg);
	void process_script(TCPSocket client, const char* msg);
	void process_command(TCPSocket client, const char* msg);

private:

	TCPSocket m_server;
	ClientArray m_clients;
};

/// Functions for accessing global console.
namespace console_server_globals
{
	// Creates the global console server.
	void init(uint16_t port, bool wait);

	/// Destroys the global console server.
	void shutdown();

	/// Updates the global console server.
	void update();

	/// Returns the global console server object.
	ConsoleServer& console();
} // namespace console_server_globals
} // namespace crown
