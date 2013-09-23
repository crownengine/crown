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

#include <stdio.h>
#include "RPCHandler.h"
#include "RPCServer.h"
#include "ProxyAllocator.h"
#include "StringUtils.h"
#include "TempAllocator.h"
#include "StringStream.h"
#include "Device.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
RPCCommandHandler::RPCCommandHandler()
	: RPCHandler("command")
{
}

//-----------------------------------------------------------------------------
void RPCCommandHandler::execute_command(RPCServer* server, ClientId client, const char* message)
{
	(void)server;
	(void)client;

	JSONParser parser(message);
	JSONElement root = parser.root();
	JSONElement command = root.key_or_nil("command");

	if (!command.is_nil())
	{
		const char* cmd = command.string_value();

		if (string::strcmp("reload", cmd) == 0)
		{
			Log::d("Unimplemented");
		}
		else if (string::strcmp("pause", cmd) == 0)
		{
			device()->pause();
		}
		else if (string::strcmp("unpause", cmd) == 0)
		{
			device()->unpause();
		}
	}
}

//-----------------------------------------------------------------------------
RPCScriptHandler::RPCScriptHandler()
	: RPCHandler("script")
{
}

//-----------------------------------------------------------------------------
void RPCScriptHandler::execute_command(RPCServer* server, ClientId client, const char* message)
{
	(void)server;
	(void)client;

	JSONParser parser(message);
	JSONElement root = parser.root();

	const char* script = root.key("script").string_value();
	device()->lua_environment()->execute_string(script);
}

//-----------------------------------------------------------------------------
RPCStatsHandler::RPCStatsHandler()
	: RPCHandler("stats")
{
}

//-----------------------------------------------------------------------------
void RPCStatsHandler::execute_command(RPCServer* server, ClientId client, const char* message)
{
	(void)message;

	TempAllocator2048 alloc;
	StringStream response(alloc);

	response << "{ \"type\" : \"response\", ";

	ProxyAllocator* proxy = ProxyAllocator::begin();

	while (proxy != NULL)
	{
		response << "\"allocator\" : \"" << proxy->name() << "\",";
		response << "\"allocated_size\" : \"" << proxy->allocated_size() << "\"";

		proxy = ProxyAllocator::next(proxy);
	}

	response << " }";

	(void)message;
	server->send_message_to(client, response.c_str());
}

//-----------------------------------------------------------------------------
RPCPingHandler::RPCPingHandler()
	: RPCHandler("ping")
{
}

//-----------------------------------------------------------------------------
void RPCPingHandler::execute_command(RPCServer* server, ClientId client, const char* message)
{
	(void)message;
	server->send_message_to(client, "{\"type\":\"pong\"}");
}

} // namespace crown
