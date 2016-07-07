/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
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
static void console_command_script(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);
	sjson::parse(json, obj);
	sjson::parse_string(obj["script"], script);
	device()->lua_environment()->execute_string(script.c_str());
}

static void console_command_reload(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	DynamicString type(ta);
	DynamicString name(ta);
	sjson::parse_string(obj["resource_type"], type);
	sjson::parse_string(obj["resource_name"], name);
	logi("Reloading resource '%s.%s'", name.c_str(), type.c_str());
	device()->reload(ResourceId(type.c_str()), ResourceId(name.c_str()));
	logi("Reloaded resource '%s.%s'", name.c_str(), type.c_str());
}

static void console_command_pause(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* /*json*/)
{
	device()->pause();
}

static void console_command_unpause(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* /*json*/)
{
	device()->unpause();
}

static void console_command_compile(ConsoleServer& cs, TCPSocket client, const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	DynamicString id(ta);
	DynamicString bundle_dir(ta);
	DynamicString platform(ta);
	sjson::parse_string(obj["id"], id);
	sjson::parse_string(obj["bundle_dir"], bundle_dir);
	sjson::parse_string(obj["platform"], platform);

	{
		TempAllocator512 ta;
		StringStream ss(ta);
		ss << "{\"type\":\"compile\",\"id\":\"" << id.c_str() << "\",\"start\":true}";
		cs.send(client, string_stream::c_str(ss));
	}

	logi("Compiling '%s'", id.c_str());
	bool succ = device()->data_compiler()->compile(bundle_dir.c_str(), platform.c_str());

	if (succ)
		logi("Compiled '%s'", id.c_str());
	else
		loge("Error while compiling '%s'", id.c_str());

	{
		TempAllocator512 ta;
		StringStream ss(ta);
		ss << "{\"type\":\"compile\",\"id\":\"" << id.c_str() << "\",\"success\":" << (succ ? "true" : "false") << "}";
		cs.send(client, string_stream::c_str(ss));
	}
}

void load_console_api(ConsoleServer& cs)
{
	cs.register_command("script",  console_command_script);
	cs.register_command("reload",  console_command_reload);
	cs.register_command("pause",   console_command_pause);
	cs.register_command("unpause", console_command_unpause);
	cs.register_command("compile", console_command_compile);
}

} // namespace crown
