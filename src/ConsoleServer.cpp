#include "ConsoleServer.h"
#include "Log.h"
#include "StringUtils.h"
#include "Device.h"

namespace crown
{

//-----------------------------------------------------------------------------
ConsoleServer::ConsoleServer()
{
}

//-----------------------------------------------------------------------------
void ConsoleServer::start(uint32_t port)
{
	m_socket.open(port);
}

//-----------------------------------------------------------------------------
void ConsoleServer::stop()
{
	m_socket.close();
}

//-----------------------------------------------------------------------------
void ConsoleServer::receive_command()
{
	// legge socket
	int32_t bytes_read = m_socket.receive((void*)data, 1024);
	if (bytes_read > 0)
	{
		parse_command();
	}
}

//-----------------------------------------------------------------------------
void ConsoleServer::parse_command()
{
	char* command = (char*)data;

	if (string::strcmp(command, "device stop") == 0)
	{
		device()->stop();
	}
}

}