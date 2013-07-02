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

#include "Device.h"
#include "OsWindow.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"
#include "StringUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_show(lua_State* L)
{
	LuaStack stack(L);

	device()->window()->show();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_hide(lua_State* L)
{
	LuaStack stack(L);

	device()->window()->hide();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_get_size(lua_State* L)
{
	LuaStack stack(L);

	uint32_t w, h;

	device()->window()->get_size(w, h);

	stack.push_uint32(w);
	stack.push_uint32(h);

	return 2;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_get_position(lua_State* L)
{
	LuaStack stack(L);

	uint32_t x, y;

	device()->window()->get_position(x, y);

	stack.push_uint32(x);
	stack.push_uint32(y);

	return 2;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_resize(lua_State* L)
{
	LuaStack stack(L);

	const int32_t w = stack.get_int(1);
	const int32_t h = stack.get_int(2);

	device()->window()->resize(w, h);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_move(lua_State* L)
{
	LuaStack stack(L);

	const int32_t x = stack.get_int(1);
	const int32_t y = stack.get_int(2);

	device()->window()->move(x, y);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_show_cursor(lua_State* L)
{
	LuaStack stack(L);

	device()->window()->show_cursor();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_hide_cursor(lua_State* L)
{
	LuaStack stack(L);

	device()->window()->hide_cursor();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_get_cursor_xy(lua_State* L)
{
	LuaStack stack(L);

	int32_t x, y;

	device()->window()->get_cursor_xy(x, y);

	stack.push_int32(x);
	stack.push_int32(y);

	return 2;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_set_cursor_xy(lua_State* L)
{
	LuaStack stack(L);

	const int32_t x = stack.get_int(1);
	const int32_t y = stack.get_int(2);

	device()->window()->set_cursor_xy(x, y);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_title(lua_State* L)
{
	LuaStack stack(L);

	const char* title = device()->window()->title();

	stack.push_string(title, string::strlen(title));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t window_set_title(lua_State* L)
{
	LuaStack stack(L);

	const char* title = stack.get_string(1);

	device()->window()->set_title(title);

	return 0;
}

//-----------------------------------------------------------------------------
void load_window(LuaEnvironment& env)
{
	env.load_module_function("Window", "show",			window_show);
	env.load_module_function("Window", "hide",			window_hide);
	env.load_module_function("Window", "get_size",		window_get_size);
	env.load_module_function("Window", "get_position",	window_get_position);
	env.load_module_function("Window", "resize",		window_resize);
	env.load_module_function("Window", "move",			window_move);
	env.load_module_function("Window", "show_cursor",	window_show_cursor);
	env.load_module_function("Window", "hide_cursor",	window_hide_cursor);
	env.load_module_function("Window", "get_cursor_xy",	window_get_cursor_xy);
	env.load_module_function("Window", "set_cursor_xy",	window_set_cursor_xy);
	env.load_module_function("Window", "title",			window_title);
	env.load_module_function("Window", "set_title",		window_set_title);
}

} // namespace crown