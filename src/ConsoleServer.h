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

#include "Types.h"
#include "TCPSocket.h"
#include "Thread.h"
#include "Mutex.h"

namespace crown
{

struct ConsoleCommand
{
	char	command[1024];
};

/// ConsoleServer runs scripts that provide some utilities for the console
class ConsoleServer
{
public:

	/// Constructor
							ConsoleServer();
	/// Start listening on @port
	void					init();
	/// Stop listening
	void					shutdown();
	/// Read-evaluation loop, executed on a different thread
	void					read_eval_loop();
	/// Execute commands, executed on main thread
	void					execute();
	/// Send data to client
	void					send(const void* data, size_t size = 1024);
	/// Receive data to client
	void					receive(char* data, size_t size = 1024);

private:

	static void*			background_thread(void* thiz);

private:

	os::TCPSocket			m_socket;

	os::Thread				m_thread;
	os::Mutex				m_command_mutex;

	// Is console active?
	bool					m_active;
	// Commands buffer
	char					m_cmd_buffer[1024];

	char 					m_err_buffer[1024];

	/// Lua script which initializes ConsoleServer
	static const char*		init_console;
};

} // namespace crown