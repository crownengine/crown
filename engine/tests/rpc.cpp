#include "Crown.h"
#include "RPCServer.h"
#include "RPCHandler.h"

using namespace crown;

// void script_callback(TCPClient* client, const char* message)
// {
// 	Log::i("Executing script.");
// }

// void command_callback(TCPClient* client, const char* message)
// {
// 	Log::i("Executing command.");
// }

int main()
{
	RPCServer rpc;

	// rpc.add_callback("script", script_callback);
	// rpc.add_callback("command", command_callback);

	// const char* json_object = "{\"type\":\"ciao\"}";
	// JSONParser parser(json_object);
	// JSONElement root = parser.root();
	// Log::i("keys: %d", root.size());
	// Log::i("type: %s", root.key("type").string_value());

	HeapAllocator heap;
	ProxyAllocator def_proxy("default-heap", default_allocator());
	ProxyAllocator proxy("main-heap", heap);

	proxy.allocate(100);

	RPCScriptHandler script;
	RPCPingHandler ping;
	RPCStatsHandler stats;

	rpc.add_handler(&script);
	rpc.add_handler(&ping);
	rpc.add_handler(&stats);

	rpc.init(true);

	while (1)
	{
		rpc.update();
		rpc.execute_callbacks();
	}

	rpc.shutdown();

	return 0;
}
