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
#include "debug_line.h"
#include "vector3.h"
#include "color4.h"

namespace crown
{

static int debug_line_add_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_line(Color4::RED, stack.get_vector3(2), stack.get_vector3(3));
	return 0;
}

static int debug_line_add_sphere(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_sphere(Color4::RED, stack.get_vector3(2), stack.get_float(3));
	return 0;
}

static int debug_line_add_obb(lua_State* L)
{
	LuaStack stack(L);
	stack.get_debug_line(1)->add_obb(Color4::RED, stack.get_matrix4x4(2), stack.get_vector3(3));
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
