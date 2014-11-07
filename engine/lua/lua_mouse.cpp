/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "input.h"
#include "mouse.h"

namespace crown
{
using namespace input_globals;

static int mouse_button_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(mouse().button_pressed((MouseButton::Enum) stack.get_int(1)));
	return 1;
}

static int mouse_button_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(mouse().button_released((MouseButton::Enum) stack.get_int(1)));
	return 1;
}

static int mouse_any_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(mouse().any_pressed());
	return 1;
}

static int mouse_any_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(mouse().any_released());
	return 1;
}

static int mouse_cursor_xy(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(mouse().cursor_xy());
	return 1;
}

static int mouse_set_cursor_xy(lua_State* L)
{
	LuaStack stack(L);
	mouse().set_cursor_xy(stack.get_vector2(1));
	return 0;
}

static int mouse_cursor_relative_xy(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(mouse().cursor_relative_xy());
	return 1;
}

static int mouse_set_cursor_relative_xy(lua_State* L)
{
	LuaStack stack(L);
	mouse().set_cursor_relative_xy(stack.get_vector2(1));
	return 0;
}

static int mouse_wheel(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(mouse().wheel());
	return 1;
}

void load_mouse(LuaEnvironment& env)
{
	env.load_module_function("Mouse", "button_pressed",         mouse_button_pressed);
	env.load_module_function("Mouse", "button_released",        mouse_button_released);
	env.load_module_function("Mouse", "any_pressed",            mouse_any_pressed);
	env.load_module_function("Mouse", "any_released",           mouse_any_released);
	env.load_module_function("Mouse", "cursor_xy",              mouse_cursor_xy);
	env.load_module_function("Mouse", "set_cursor_xy",          mouse_set_cursor_xy);
	env.load_module_function("Mouse", "cursor_relative_xy",     mouse_cursor_relative_xy);
	env.load_module_function("Mouse", "set_cursor_relative_xy", mouse_set_cursor_relative_xy);
	env.load_module_function("Mouse", "wheel",                  mouse_wheel);
	
	env.load_module_enum("Mouse", "LEFT",   MouseButton::LEFT);
	env.load_module_enum("Mouse", "MIDDLE", MouseButton::MIDDLE);
	env.load_module_enum("Mouse", "RIGHT",  MouseButton::RIGHT);
}

} // namespace crown
