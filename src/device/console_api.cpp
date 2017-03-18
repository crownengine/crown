/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "console_server.h"
#include "data_compiler.h"
#include "device.h"
#include "dynamic_string.h"
#include "json_object.h"
#include "lua_environment.h"
#include "sjson.h"
#include "string_stream.h"
#include "temp_allocator.h"

namespace crown
{
static void console_command_script(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json, void* /*user_data*/)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(json, obj);
	sjson::parse_string(obj["script"], script);

	device()->_lua_environment->execute_string(script.c_str());
}

static void console_command(ConsoleServer& cs, TCPSocket client, const char* json, void* /*user_data*/)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	JsonArray args(ta);

	sjson::parse(json, obj);
	sjson::parse_array(obj["args"], args);

	DynamicString cmd(ta);
	sjson::parse_string(args[0], cmd);

	if (cmd == "pause")
		device()->pause();
	else if (cmd == "unpause")
		device()->unpause();
	else if (cmd == "reload")
	{
		if (array::size(args) != 3)
		{
			cs.error(client, "Usage: reload type name");
			return;
		}

		DynamicString type(ta);
		DynamicString name(ta);
		sjson::parse_string(args[1], type);
		sjson::parse_string(args[2], name);

		device()->reload(ResourceId(type.c_str()), ResourceId(name.c_str()));
	}
}

void load_console_api(ConsoleServer& cs)
{
	cs.register_command("script",  console_command_script, NULL);
	cs.register_command("command", console_command, NULL);
}

} // namespace crown
