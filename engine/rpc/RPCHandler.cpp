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

#include "RPCHandler.h"
#include "RPCServer.h"
#include "ProxyAllocator.h"
#include "StringUtils.h"
#include <stdio.h>
#include "TempAllocator.h"
#include "StringStream.h"

namespace crown
{

//-----------------------------------------------------------------------------
RPCScriptHandler::RPCScriptHandler()
	: RPCHandler("script")
{
}

//-----------------------------------------------------------------------------
void RPCScriptHandler::execute_command(RPCServer* server, TCPClient* client, const char* message)
{
	(void)server;
	(void)client;

	JSONParser parser(message);
	JSONElement root = parser.root();

	const char* script = root.key("script").string_value();
	Log::i("RPCScritpHandler: %s", script);
}

//-----------------------------------------------------------------------------
RPCStatsHandler::RPCStatsHandler()
	: RPCHandler("stats")
{
}

//-----------------------------------------------------------------------------
void RPCStatsHandler::execute_command(RPCServer* server, TCPClient* client, const char* message)
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
void RPCPingHandler::execute_command(RPCServer* server, TCPClient* client, const char* message)
{
	(void)message;
	server->send_message_to(client, "{\"type\":\"pong\"}");
}

} // namespace crown
