/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "input.h"
#include "touch.h"

namespace crown
{
using namespace input_globals;

static int touch_pointer_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(touch().pointer_down((uint8_t) stack.get_int(1)));
	return 1;
}

static int touch_pointer_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(touch().pointer_up((uint8_t) stack.get_int(1)));
	return 1;
}

static int touch_any_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(touch().any_down());
	return 1;
}

static int touch_any_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(touch().any_up());
	return 1;
}

static int touch_pointer_xy(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(touch().pointer_xy((uint8_t) stack.get_int(1)));
	return 1;
}

void load_touch(LuaEnvironment& env)
{
	env.load_module_function("Touch", "pointer_down", touch_pointer_down);
	env.load_module_function("Touch", "pointer_up",   touch_pointer_up);
	env.load_module_function("Touch", "any_down",     touch_any_down);
	env.load_module_function("Touch", "any_up",       touch_any_up);
	env.load_module_function("Touch", "pointer_xy",   touch_pointer_xy);
}

} // namespace crown
