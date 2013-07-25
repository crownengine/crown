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

static IntSetting g_port("read_port", "port used for reading", 10000, 9999, 65535);

//-----------------------------------------------------------------------------
ConsoleServer::ConsoleServer() :
	m_thread(ConsoleServer::background_thread, (void*)this, "console-thread"),
	m_active(false)
{
	string::strncpy(m_cmd_buffer, "", 1024);
	string::strncpy(m_err_buffer, "", 1024);
}

//-----------------------------------------------------------------------------
void ConsoleServer::init()
{
	m_active = true;
}

//-----------------------------------------------------------------------------
void ConsoleServer::shutdown()
{
	m_active = false;

	m_socket.close();
}

//-----------------------------------------------------------------------------
void ConsoleServer::read_eval_loop()
{
	m_socket.open(g_port);
	
	char cmd[1024];

	while (m_active)
	{
		string::strncpy(cmd, "", 1024);
		receive((char*)cmd, 1024);

		// Fill command buffer
		string::strncpy(m_cmd_buffer, cmd, 1024);
	}

	Log::i("End read-eval loop");
	m_socket.close();
}

//-----------------------------------------------------------------------------
void ConsoleServer::execute()
{
	m_command_mutex.lock();

	LuaEnvironment* lua = device()->lua_environment();

	lua->load_buffer(m_cmd_buffer, string::strlen(m_cmd_buffer));
	lua->execute(0, 0);

	string::strncpy(m_cmd_buffer, "", 1024);

	string::strncpy(m_err_buffer, lua->error(),  1024);

	if (string::strcmp(m_err_buffer, "") != 0)
	{
		// Fill error buffer
		send((char*)m_err_buffer, 1024);
	}

	string::strncpy(m_err_buffer, "", 1024);

	m_command_mutex.unlock();
}

//-----------------------------------------------------------------------------
void ConsoleServer::send(const void* data, size_t size)
{
	m_socket.send(data, size);
}

//-----------------------------------------------------------------------------
void ConsoleServer::receive(char* data, size_t size)
{
	m_socket.receive(data, size);
}

//-----------------------------------------------------------------------------
void* ConsoleServer::background_thread(void* thiz)
{
	((ConsoleServer*)thiz)->read_eval_loop();

	return NULL;
}


} // namespace crown