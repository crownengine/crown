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

#include "config.h"

#if CROWN_PLATFORM_LINUX
	#include "os_window_linux.h"
#elif CROWN_PLATFORM_WINDOWS
	#include "os_window_windows.h"
#elif CROWN_PLATFORM_ANDROID
	#include "os_window_android.h"
#endif

#include "device.h"
#include "lua_stack.h"
#include "lua_environment.h"
#include "main.h"

namespace crown
{

static int window_show(lua_State* L)
{
	LuaStack stack(L);
	// window()->show();
	return 0;
}

static int window_hide(lua_State* L)
{
	LuaStack stack(L);
	// window()->hide();
	return 0;
}

static int window_resize(lua_State* L)
{
	LuaStack stack(L);
	// window()->resize(stack.get_int(1), stack.get_int(2));
	return 0;
}

static int window_move(lua_State* L)
{
	LuaStack stack(L);
	// window()->move(stack.get_int(1), stack.get_int(2));
	return 0;
}

static int window_minimize(lua_State* /*L*/)
{
	// window()->minimize();
	return 0;
}

static int window_restore(lua_State* /*L*/)
{
	// window()->restore();
	return 0;
}

static int window_is_resizable(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(/*window()->is_resizable()*/ false);
	return 1;
}

static int window_set_resizable(lua_State* L)
{
	LuaStack stack(L);
	// window()->set_resizable(stack.get_bool(1));
	return 0;
}

static int window_title(lua_State* L)
{
	LuaStack stack(L);
	// stack.push_string(window()->title());
	stack.push_string("");
	return 1;
}

static int window_set_title(lua_State* L)
{
	LuaStack stack(L);
	// window()->set_title(stack.get_string(1));
	return 0;
}

void load_window(LuaEnvironment& env)
{
	env.load_module_function("Window", "show",          window_show);
	env.load_module_function("Window", "hide",          window_hide);
	env.load_module_function("Window", "resize",        window_resize);
	env.load_module_function("Window", "move",          window_move);
	env.load_module_function("Window", "minimize",      window_minimize);
	env.load_module_function("Window", "restore",       window_restore);
	env.load_module_function("Window", "is_resizable",  window_is_resizable);
	env.load_module_function("Window", "set_resizable", window_set_resizable);
	env.load_module_function("Window", "title",         window_title);
	env.load_module_function("Window", "set_title",     window_set_title);
}

} // namespace crown
