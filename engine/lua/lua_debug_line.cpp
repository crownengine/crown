/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "debug_line.h"
#include "vector3.h"
#include "color4.h"

namespace crown
{

static int debug_line_add_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_line(stack.get_vector3(2), stack.get_vector3(3), stack.get_color4(4));
	return 0;
}

static int debug_line_add_sphere(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_sphere(stack.get_vector3(2), stack.get_float(3), stack.get_color4(4));
	return 0;
}

static int debug_line_add_obb(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_obb(stack.get_matrix4x4(2), stack.get_vector3(3), stack.get_color4(4));
	return 0;
}

static int debug_line_clear(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->clear();
	return 0;
}

static int debug_line_commit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->commit();
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
	env.load_module_function("DebugLine", "add_sphere", debug_line_add_sphere);
	env.load_module_function("DebugLine", "add_obb",    debug_line_add_obb);
	env.load_module_function("DebugLine", "clear",      debug_line_clear);
	env.load_module_function("DebugLine", "commit",     debug_line_commit);
	env.load_module_function("DebugLine", "__index",    "DebugLine");
	env.load_module_function("DebugLine", "__tostring", debug_line_tostring);
}

} // namespace crown
