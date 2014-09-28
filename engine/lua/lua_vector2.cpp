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

#include "vector2.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int vector2_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(Vector2(stack.get_float(1), stack.get_float(2)));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector2_new(L);
}

//-----------------------------------------------------------------------------
static int vector2_x(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector2(1).x);
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_y(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector2(1).y);
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_set_x(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector2(1).x = stack.get_float(2);
	return 0;
}

//-----------------------------------------------------------------------------
static int vector2_set_y(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector2(1).y = stack.get_float(2);
	return 0;
}

//-----------------------------------------------------------------------------
static int vector2_values(lua_State* L)
{
	LuaStack stack(L);
	Vector2& a = stack.get_vector2(1);
	stack.push_float(a.x);
	stack.push_float(a.y);
	return 2;
}

//-----------------------------------------------------------------------------
static int vector2_add(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(stack.get_vector2(1) + stack.get_vector2(2));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_subtract(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(stack.get_vector2(1) - stack.get_vector2(2));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(stack.get_vector2(1) * stack.get_float(2));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_divide(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(stack.get_vector2(1) / stack.get_float(2));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_dot(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector2::dot(stack.get_vector2(1), stack.get_vector2(2)));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_equal(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_vector2(1) == stack.get_vector2(2));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector2::length(stack.get_vector2(1)));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_squared_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector2::squared_length(stack.get_vector2(1)));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_set_length(lua_State* L)
{
	LuaStack stack(L);
	vector2::set_length(stack.get_vector2(1), stack.get_float(2));
	return 0;
}

//-----------------------------------------------------------------------------
static int vector2_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(vector2::normalize(stack.get_vector2(1)));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector2::distance(stack.get_vector2(1), stack.get_vector2(2)));
	return 1;
}

//-----------------------------------------------------------------------------
static int vector2_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(vector2::angle(stack.get_vector2(1), stack.get_vector2(2)));
	return 1;
}

//-----------------------------------------------------------------------------
void load_vector2(LuaEnvironment& env)
{
	env.load_module_function("Vector2", "new",            vector2_new);
	env.load_module_function("Vector2", "x",              vector2_x);
	env.load_module_function("Vector2", "y",              vector2_y);
	env.load_module_function("Vector2", "set_x",          vector2_set_x);
	env.load_module_function("Vector2", "set_y",          vector2_set_y);
	env.load_module_function("Vector2", "values",         vector2_values);
	env.load_module_function("Vector2", "add",            vector2_add);
	env.load_module_function("Vector2", "subtract",       vector2_subtract);
	env.load_module_function("Vector2", "multiply",       vector2_multiply);
	env.load_module_function("Vector2", "divide",         vector2_divide);
	env.load_module_function("Vector2", "dot",            vector2_dot);
	env.load_module_function("Vector2", "equal",          vector2_equal);
	env.load_module_function("Vector2", "length",         vector2_length);
	env.load_module_function("Vector2", "squared_length", vector2_squared_length);
	env.load_module_function("Vector2", "set_length",     vector2_set_length);
	env.load_module_function("Vector2", "normalize",      vector2_normalize);
	env.load_module_function("Vector2", "distance",       vector2_distance);
	env.load_module_function("Vector2", "angle",          vector2_angle);

	env.load_module_constructor("Vector2", vector2_ctor);
}

} // namespace crown
