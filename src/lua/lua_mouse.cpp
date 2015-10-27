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

static int mouse_name(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->input_manager()->mouse()->name());
	return 1;
}

static int mouse_num_buttons(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->mouse()->num_buttons());
	return 1;
}

static int mouse_num_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->mouse()->num_axes());
	return 1;
}

static int mouse_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->mouse()->pressed((MouseButton::Enum) stack.get_int(1)));
	return 1;
}

static int mouse_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->mouse()->released((MouseButton::Enum) stack.get_int(1)));
	return 1;
}

static int mouse_any_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->mouse()->any_pressed());
	return 1;
}

static int mouse_any_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->mouse()->any_released());
	return 1;
}

static int mouse_axis(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(device()->input_manager()->mouse()->axis(stack.get_int(1)));
	return 1;
}

void load_mouse(LuaEnvironment& env)
{
	env.load_module_function("Mouse", "name",         mouse_name);
	env.load_module_function("Mouse", "num_buttons",  mouse_num_buttons);
	env.load_module_function("Mouse", "num_axes",     mouse_num_axes);
	env.load_module_function("Mouse", "pressed",      mouse_pressed);
	env.load_module_function("Mouse", "released",     mouse_released);
	env.load_module_function("Mouse", "any_pressed",  mouse_any_pressed);
	env.load_module_function("Mouse", "any_released", mouse_any_released);
	env.load_module_function("Mouse", "axis",         mouse_axis);

	env.load_module_enum("Mouse", "LEFT",   MouseButton::LEFT);
	env.load_module_enum("Mouse", "MIDDLE", MouseButton::MIDDLE);
	env.load_module_enum("Mouse", "RIGHT",  MouseButton::RIGHT);
}

} // namespace crown
