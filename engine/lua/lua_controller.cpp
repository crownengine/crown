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

#include "controller.h"
#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int controller_move(lua_State* L)
{
	LuaStack stack(L);
	stack.get_controller(1)->move(stack.get_vector3(2));
	return 0;
}

//-----------------------------------------------------------------------------
static int controller_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_controller(1)->position());
	return 1;
}

//-----------------------------------------------------------------------------
static int controller_collides_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_controller(1)->collides_up());
	return 1;
}

//-----------------------------------------------------------------------------
static int controller_collides_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_controller(1)->collides_down());
	return 1;
}

//-----------------------------------------------------------------------------
static int controller_collides_sides(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_controller(1)->collides_sides());
	return 1;
}

//-----------------------------------------------------------------------------
void load_controller(LuaEnvironment& env)
{
	env.load_module_function("Controller", "move",           controller_move);
	env.load_module_function("Controller", "position",       controller_position);
	env.load_module_function("Controller", "collides_up",    controller_collides_up);
	env.load_module_function("Controller", "collides_down",  controller_collides_down);
	env.load_module_function("Controller", "collides_sides", controller_collides_sides);
}

} // namespace crown
