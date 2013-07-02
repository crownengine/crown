/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "LuaStack.h"
#include "Device.h"
#include "LuaEnvironment.h"
#include "Mouse.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mouse_button_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t button = stack.get_int(1);

	stack.push_bool(device()->mouse()->button_pressed((MouseButton) button));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mouse_button_released(lua_State* L)
{
	LuaStack stack(L);

	int32_t button = stack.get_int(1);

	stack.push_bool(device()->mouse()->button_released((MouseButton) button));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mouse_cursor_xy(lua_State* L)
{
	LuaStack stack(L);

	Vec2* xy = next_vec2();

	*xy = device()->mouse()->cursor_xy();

	stack.push_vec2(xy);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mouse_set_cursor_xy(lua_State* L)
{
	LuaStack stack(L);

	Vec2* xy = stack.get_vec2(1);

	device()->mouse()->set_cursor_xy(*xy);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mouse_cursor_relative_xy(lua_State* L)
{
	LuaStack stack(L);

	Vec2* xy = next_vec2();
	*xy = device()->mouse()->cursor_relative_xy();

	stack.push_vec2(xy);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mouse_set_cursor_relative_xy(lua_State* L)
{
	LuaStack stack(L);

	Vec2* xy = (Vec2*) stack.get_vec2(1);

	device()->mouse()->set_cursor_relative_xy(*xy);

	return 0;
}

//-----------------------------------------------------------------------------
void load_mouse(LuaEnvironment& env)
{
	env.load_module_function("Mouse", "button_pressed",			mouse_button_pressed);
	env.load_module_function("Mouse", "button_released",		mouse_button_released);
	env.load_module_function("Mouse", "cursor_xy",				mouse_cursor_xy);
	env.load_module_function("Mouse", "set_cursor_xy",			mouse_set_cursor_xy);
	env.load_module_function("Mouse", "cursor_relative_xy",		mouse_cursor_relative_xy);
	env.load_module_function("Mouse", "set_cursor_relative_xy",	mouse_set_cursor_relative_xy);
}

} // namespace crown
