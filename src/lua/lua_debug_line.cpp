/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "debug_line.h"

namespace crown
{

static int debug_line_add_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_line(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_color4(4)
		);
	return 0;
}

static int debug_line_add_axes(lua_State* L)
{
	LuaStack stack(L);
	const float length = stack.num_args() == 3 ? stack.get_float(3) : 1.0f;
	stack.get_debug_line(1)->add_axes(stack.get_matrix4x4(2), length);
	return 0;
}

static int debug_line_add_circle(lua_State* L)
{
	LuaStack stack(L);
	const uint32_t segments = stack.num_args() >= 6 ? stack.get_int(6) : 36;
	stack.get_debug_line(1)->add_circle(stack.get_vector3(2)
		, stack.get_float(3)
		, stack.get_vector3(4)
		, stack.get_color4(5)
		, segments
		);
	return 0;
}

static int debug_line_add_cone(lua_State* L)
{
	LuaStack stack(L);
	const uint32_t segments = stack.num_args() >= 6 ? stack.get_int(6) : 36;
	stack.get_debug_line(1)->add_cone(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, stack.get_color4(5)
		, segments
		);
	return 0;
}

static int debug_line_add_sphere(lua_State* L)
{
	LuaStack stack(L);
	const uint32_t segments = stack.num_args() >= 5 ? stack.get_int(5) : 36;
	stack.get_debug_line(1)->add_sphere(stack.get_vector3(2)
		, stack.get_float(3)
		, stack.get_color4(4)
		, segments
		);
	return 0;
}

static int debug_line_add_obb(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_obb(stack.get_matrix4x4(2)
		, stack.get_vector3(3)
		, stack.get_color4(4)
		);
	return 0;
}

static int debug_line_reset(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->reset();
	return 0;
}

static int debug_line_submit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->submit();
	return 0;
}

static int debug_line_tostring(lua_State* L)
{
	LuaStack stack(L);
	stack.push_fstring("DebugLine (%p)", stack.get_debug_line(1));
	return 1;
}

void load_debug_line(LuaEnvironment& env)
{
	env.load_module_function("DebugLine", "add_line",   debug_line_add_line);
	env.load_module_function("DebugLine", "add_axes",   debug_line_add_axes);
	env.load_module_function("DebugLine", "add_circle", debug_line_add_circle);
	env.load_module_function("DebugLine", "add_cone",   debug_line_add_cone);
	env.load_module_function("DebugLine", "add_sphere", debug_line_add_sphere);
	env.load_module_function("DebugLine", "add_obb",    debug_line_add_obb);
	env.load_module_function("DebugLine", "reset",      debug_line_reset);
	env.load_module_function("DebugLine", "submit",     debug_line_submit);
	env.load_module_function("DebugLine", "__index",    "DebugLine");
	env.load_module_function("DebugLine", "__tostring", debug_line_tostring);
}

} // namespace crown
