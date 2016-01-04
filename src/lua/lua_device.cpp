/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "device.h"
#include "resource_package.h"
#include "world.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "temp_allocator.h"
#include "string_stream.h"
#include "console_server.h"
#include "resource_manager.h"
#include "profiler.h"

namespace crown
{

static int device_platform(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->platform());
	return 1;
}

static int device_architecture(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->architecture());
	return 1;
}

static int device_version(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->version());
	return 1;
}

static int device_last_delta_time(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(device()->last_delta_time());
	return 1;
}

static int device_quit(lua_State* /*L*/)
{
	device()->quit();
	return 0;
}

static int device_resolution(lua_State* L)
{
	LuaStack stack(L);
	uint16_t w, h;
	device()->resolution(w, h);
	stack.push_int(w);
	stack.push_int(h);
	return 2;
}

static int device_create_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_world(device()->create_world());
	return 1;
}

static int device_destroy_world(lua_State* L)
{
	LuaStack stack(L);
	device()->destroy_world(*stack.get_world(1));
	return 0;
}

static int device_render_world(lua_State* L)
{
	LuaStack stack(L);
	device()->render_world(*stack.get_world(1), stack.get_camera(2));
	return 0;
}

static int device_create_resource_package(lua_State* L)
{
	LuaStack stack(L);
	stack.push_resource_package(device()->create_resource_package(stack.get_resource_id(1)));
	return 1;
}

static int device_destroy_resource_package(lua_State* L)
{
	LuaStack stack(L);
	device()->destroy_resource_package(*stack.get_resource_package(1));
	return 0;
}

static int device_console_send(lua_State* L)
{
	using namespace string_stream;
	LuaStack stack(L);

	TempAllocator1024 alloc;
	StringStream json(alloc);

	json << "{";
	/* table is in the stack at index 'i' */
	stack.push_nil();  /* first key */
	while (stack.next(1) != 0)
	{
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		json << "\"" << stack.get_string(-2) << "\":\"" << stack.get_string(-1) << "\",";
		/* removes 'value'; keeps 'key' for next iteration */
		stack.pop(1);
	}
	/* pop key */
	stack.pop(1);
	json << "}";

	console_server_globals::console()->send(c_str(json));
	return 0;
}

static int device_can_get(lua_State* L)
{
	LuaStack stack(L);
	const StringId64 type(stack.get_string(1));
	stack.push_bool(device()->resource_manager()->can_get(type, stack.get_resource_id(2)));
	return 1;
}

static int device_enable_resource_autoload(lua_State* L)
{
	LuaStack stack(L);
	device()->resource_manager()->enable_autoload(stack.get_bool(1));
	return 0;
}

static int profiler_enter_scope(lua_State* L)
{
	LuaStack stack(L);
	profiler::enter_profile_scope(stack.get_string(1));
	return 0;
}

static int profiler_leave_scope(lua_State* L)
{
	LuaStack stack(L);
	profiler::leave_profile_scope();
	return 0;
}

static int profiler_record(lua_State* L)
{
	LuaStack stack(L);

	const char* name = stack.get_string(1);

	if (stack.is_number(2))
		profiler::record_float(name, stack.get_float(2));
	else
		profiler::record_vector3(name, stack.get_vector3(2));

	return 0;
}

void load_device(LuaEnvironment& env)
{
	env.load_module_function("Device", "platform",                 device_platform);
	env.load_module_function("Device", "architecture",             device_architecture);
	env.load_module_function("Device", "version",                  device_version);
	env.load_module_function("Device", "last_delta_time",          device_last_delta_time);
	env.load_module_function("Device", "quit",                     device_quit);
	env.load_module_function("Device", "resolution",               device_resolution);
	env.load_module_function("Device", "create_world",             device_create_world);
	env.load_module_function("Device", "destroy_world",            device_destroy_world);
	env.load_module_function("Device", "render_world",             device_render_world);
	env.load_module_function("Device", "create_resource_package",  device_create_resource_package);
	env.load_module_function("Device", "destroy_resource_package", device_destroy_resource_package);
	env.load_module_function("Device", "console_send",             device_console_send);
	env.load_module_function("Device", "can_get",                  device_can_get);
	env.load_module_function("Device", "enable_resource_autoload", device_enable_resource_autoload);

	env.load_module_function("Profiler", "enter_scope", profiler_enter_scope);
	env.load_module_function("Profiler", "leave_scope", profiler_leave_scope);
	env.load_module_function("Profiler", "record",      profiler_record);
}

} // namespace crown
