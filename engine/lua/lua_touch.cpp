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

#include "lua_stack.h"
#include "lua_environment.h"
#include "input.h"
#include "touch.h"

namespace crown
{
using namespace input_globals;

//-----------------------------------------------------------------------------
static int touch_pointer_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(touch().pointer_down((uint8_t) stack.get_int(1)));
	return 1;
}

//-----------------------------------------------------------------------------
static int touch_pointer_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(touch().pointer_up((uint8_t) stack.get_int(1)));
	return 1;
}

//-----------------------------------------------------------------------------
static int touch_any_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(touch().any_down());
	return 1;
}

//-----------------------------------------------------------------------------
static int touch_any_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(touch().any_up());
	return 1;
}

//-----------------------------------------------------------------------------
static int touch_pointer_xy(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(touch().pointer_xy((uint8_t) stack.get_int(1)));
	return 1;
}

//-----------------------------------------------------------------------------
void load_touch(LuaEnvironment& env)
{
	env.load_module_function("Touch", "pointer_down", touch_pointer_down);
	env.load_module_function("Touch", "pointer_up",   touch_pointer_up);
	env.load_module_function("Touch", "any_down",     touch_any_down);
	env.load_module_function("Touch", "any_up",       touch_any_up);
	env.load_module_function("Touch", "pointer_xy",   touch_pointer_xy);
}

} // namespace crown
