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

#include "quaternion.h"
#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"


namespace crown
{

static int quaternion_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(Quaternion(stack.get_vector3(1), stack.get_float(2)));
	return 1;
}

static int quaternion_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return quaternion_new(L);
}

static int quaternion_negate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(-stack.get_quaternion(1));
	return 1;
}

static int quaternion_identity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion::IDENTITY);
	return 1;
}

static int quaternion_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(quaternion::length(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion::normalize(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_conjugate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion::conjugate(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_inverse(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion::inverse(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_quaternion(1) * stack.get_quaternion(2));
	return 1;
}

static int quaternion_multiply_by_scalar(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_quaternion(1) * stack.get_float(2));
	return 1;
}

static int quaternion_power(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion::power(stack.get_quaternion(1), stack.get_float(2)));
	return 1;
}

static int quaternion_elements(lua_State* L)
{
	LuaStack stack(L);
	const Quaternion& q = stack.get_quaternion(1);
	stack.push_float(q.x);
	stack.push_float(q.y);
	stack.push_float(q.z);
	stack.push_float(q.w);
	return 4;
}

void load_quaternion(LuaEnvironment& env)
{
	env.load_module_function("Quaternion", "new",                quaternion_new);
	env.load_module_function("Quaternion", "negate",             quaternion_negate);
	env.load_module_function("Quaternion", "identity",           quaternion_identity);
	env.load_module_function("Quaternion", "multiply",           quaternion_multiply);
	env.load_module_function("Quaternion", "multiply_by_scalar", quaternion_multiply_by_scalar);
	env.load_module_function("Quaternion", "length",             quaternion_length);
	env.load_module_function("Quaternion", "normalize",          quaternion_normalize);
	env.load_module_function("Quaternion", "conjugate",          quaternion_conjugate);
	env.load_module_function("Quaternion", "inverse",            quaternion_inverse);
	env.load_module_function("Quaternion", "power",              quaternion_power);
	env.load_module_function("Quaternion", "elements",           quaternion_elements);
	
	env.load_module_constructor("Quaternion", quaternion_ctor);
}

} //namespace crown
