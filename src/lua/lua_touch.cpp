/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "device.h"
#include "input_manager.h"
#include "input_device.h"

namespace crown
{

static int touch_name(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->input_manager()->touch()->name());
	return 1;
}

static int touch_num_buttons(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->touch()->num_buttons());
	return 1;
}

static int touch_num_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->touch()->num_axes());
	return 1;
}

static int touch_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->touch()->pressed(stack.get_int(1)));
	return 1;
}

static int touch_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->touch()->released(stack.get_int(1)));
	return 1;
}

static int touch_any_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->touch()->any_pressed());
	return 1;
}

static int touch_any_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->touch()->any_released());
	return 1;
}

static int touch_axis(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(device()->input_manager()->touch()->axis(stack.get_int(1)));
	return 1;
}

void load_touch(LuaEnvironment& env)
{
	env.load_module_function("Touch", "name",         touch_name);
	env.load_module_function("Touch", "num_buttons",  touch_num_buttons);
	env.load_module_function("Touch", "num_axes",     touch_num_axes);
	env.load_module_function("Touch", "pressed",      touch_pressed);
	env.load_module_function("Touch", "released",     touch_released);
	env.load_module_function("Touch", "any_pressed",  touch_any_pressed);
	env.load_module_function("Touch", "any_released", touch_any_released);
	env.load_module_function("Touch", "axis",         touch_axis);
}

} // namespace crown
