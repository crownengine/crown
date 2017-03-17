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

static void console_command_reload(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json, void* /*user_data*/)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString type(ta);
	DynamicString name(ta);

	sjson::parse(json, obj);
	sjson::parse_string(obj["resource_type"], type);
	sjson::parse_string(obj["resource_name"], name);

	device()->reload(ResourceId(type.c_str()), ResourceId(name.c_str()));
}

static void console_command_pause(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* /*json*/, void* /*user_data*/)
{
	device()->pause();
}

static void console_command_unpause(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* /*json*/, void* /*user_data*/)
{
	device()->unpause();
}

void load_console_api(ConsoleServer& cs)
{
	cs.register_command("script",  console_command_script, NULL);
	cs.register_command("reload",  console_command_reload, NULL);
	cs.register_command("pause",   console_command_pause, NULL);
	cs.register_command("unpause", console_command_unpause, NULL);
}

} // namespace crown
