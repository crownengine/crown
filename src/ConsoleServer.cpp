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
#include "Log.h"
#include "StringUtils.h"
#include "Device.h"
#include "LuaEnvironment.h"
#include "StringUtils.h"
#include "JSONParser.h"
#include "IntSetting.h"

namespace crown
{

static IntSetting g_read_port("read_port", "port used for reading", 10000, 9999, 65535);
static IntSetting g_write_port("write_port", "port used for writing", 10001, 9999, 65535);


//-----------------------------------------------------------------------------
const char* ConsoleServer::init_console = 	"cmd = ""; "
											"local i = 0; "
											"for name,class in pairs(_G) do "
											"	if type(class) == 'table' then "
			 								"		for func_name,func in pairs(class) do "
			 								"			if type(func) == 'function' then "
			 								"				i = i + 1 "
											"				cmd = cmd .. name .. '.' .. func_name "
											"			end "
											"		end "
											"	end "
											"end";


//-----------------------------------------------------------------------------
ConsoleServer::ConsoleServer() :
	m_active(false),
	m_thread(ConsoleServer::background_thread, (void*)this, "console-thread")
{
}

//-----------------------------------------------------------------------------
void ConsoleServer::init()
{
	LuaEnvironment* lua = device()->lua_environment();

	lua->load_buffer(init_console, string::strlen(init_console));
	lua->execute(0, 0);

	m_active = true;
}

//-----------------------------------------------------------------------------
void ConsoleServer::shutdown()
{
	m_active = false;
}

//-----------------------------------------------------------------------------
void ConsoleServer::read_eval_loop()
{
	m_socket.open(g_read_port);

	LuaEnvironment* lua = device()->lua_environment();

	char cmd[1024];

	while (m_active)
	{
		// FIXME: send response of previous command
		if (lua->status())
		{
			const char* tmp = lua->error();
			send((char*)tmp, 1024);
		}

		receive((char*)cmd, 1024);

		// FIXME: parse and then fill m_cmd_buffer

		string::strcpy(m_cmd_buffer, cmd);
	}

	m_socket.close();
}

//-----------------------------------------------------------------------------
void ConsoleServer::execute()
{
	m_command_mutex.lock();

	LuaEnvironment* lua = device()->lua_environment();

	lua->load_buffer(m_cmd_buffer, string::strlen(m_cmd_buffer));
	lua->execute(0, 0);
	// Reset cmd buffer
	m_cmd_buffer[0] = '\0';

	// If LuaEnvironment status is false
	if (!lua->status())
	{

		const char* tmp = lua->error();

		// Send error to client
	}

	m_command_mutex.unlock();
}

//-----------------------------------------------------------------------------
void ConsoleServer::send(const void* data, size_t size)
{
	bool sent = m_socket.send(data, size);
}

//-----------------------------------------------------------------------------
void ConsoleServer::receive(char* data, size_t size)
{
	int32_t bytes_read = m_socket.receive(data, size);

	// FIX: parse json (JSONParser needs rework)
}

//-----------------------------------------------------------------------------
void* ConsoleServer::background_thread(void* thiz)
{
	((ConsoleServer*)thiz)->read_eval_loop();	
}


} // namespace crown