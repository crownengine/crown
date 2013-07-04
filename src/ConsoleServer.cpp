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

static IntSetting g_port("console_port", "port used by console", 10000, 9999, 65535);

//-----------------------------------------------------------------------------
const char* ConsoleServer::init_console = 	"cmd = {}; "
											"local i = 0; "
											"for name,class in pairs(_G) do "
											"	if type(class) == 'table' then "
			 								"		for func_name,func in pairs(class) do "
			 								"			if type(func) == 'function' then "
			 								"				i = i + 1 "
											"				cmd[i] = name .. '.' .. func_name "
											"			end "
											"		end "
											"	end "
											"end";

//-----------------------------------------------------------------------------
const char* ConsoleServer::retrieve_cmds = 	"for i,line in pairs(cmd) do "
											"	print(i .. '- ' .. line) "
											"end";

//-----------------------------------------------------------------------------
ConsoleServer::ConsoleServer() :
	m_active(false),
	m_count(0),
	m_thread(ConsoleServer::background_thread, (void*)this, "console-thread")
{
}

//-----------------------------------------------------------------------------
void ConsoleServer::init()
{

	device()->lua_environment()->load_buffer(init_console, string::strlen(init_console));
	device()->lua_environment()->execute(0, 0);

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
	m_socket.open(g_port);

	Log::i("In read-eval loop");

	char tmp[1024];

	while (m_active)
	{
		receive((char*)tmp, 1024);

		Log::i("Received: %s", tmp);
		// FIXME: parse and then fill m_buffer

		add_command(tmp);
	}

	m_socket.close();
}

//-----------------------------------------------------------------------------
void ConsoleServer::add_command(const char* cmd)
{
	m_command_mutex.lock();

	Log::i("Pussy: %s, len: %d", cmd, string::strlen(cmd));
	string::strcpy(m_buffer[m_count].command, cmd);
	Log::i(m_buffer[m_count].command);

	++m_count;

	m_command_mutex.unlock();
}

//-----------------------------------------------------------------------------
void ConsoleServer::execute()
{
	m_command_mutex.lock();
	for (uint32_t i = 0; i < m_count; i++)
	{
		Log::i("command: %s, size: %d", m_buffer[i].command, string::strlen(m_buffer[i].command));
		device()->lua_environment()->load_buffer(m_buffer[i].command, string::strlen(m_buffer[i].command));
		device()->lua_environment()->execute(0, 0);
		string::strcpy(m_buffer[i].command, "");
	}

	m_count = 0;
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
void ConsoleServer::parse_command(const uint8_t* data)
{

}

//-----------------------------------------------------------------------------
void ConsoleServer::command_list()
{
	device()->lua_environment()->load_buffer(retrieve_cmds, string::strlen(retrieve_cmds));
	device()->lua_environment()->execute(0, 0);
}

//-----------------------------------------------------------------------------
void* ConsoleServer::background_thread(void* thiz)
{
	((ConsoleServer*)thiz)->read_eval_loop();	
}


}