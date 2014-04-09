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

#include "Quaternion.h"
#include "Vector3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int quaternionbox_new(lua_State* L)
{
	LuaStack stack(L);

	Quaternion q;
	if (stack.num_args() == 1)
	{
		q = stack.get_quaternion(1);
	}
	if (stack.num_args() == 2)
	{
		Quaternion quat(stack.get_vector3(1), stack.get_float(2));
		q = quat;
	}
	else if (stack.num_args() == 4)
	{
		Quaternion quat(stack.get_float(1), stack.get_float(2),	stack.get_float(3),	stack.get_float(4));
		q = quat;
	}

	stack.push_quaternionbox(q);
	return 1;
}

//-----------------------------------------------------------------------------
static int quaternionbox_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return quaternionbox_new(L);
}

//-----------------------------------------------------------------------------
static int quaternionbox_store(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);
	
	if (stack.num_args() == 2)
	{
		q = stack.get_quaternion(2);
	}
	if (stack.num_args() == 3)
	{
		Quaternion quat(stack.get_vector3(2), stack.get_float(3));
		q = quat;
	}
	else if (stack.num_args() == 5)
	{
		Quaternion quat(stack.get_float(2), stack.get_float(3),	stack.get_float(4),	stack.get_float(5));
		q = quat;
	}
	return 0;
}

//-----------------------------------------------------------------------------
static int quaternionbox_unbox(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	stack.push_quaternion(q);
	return 1;
}

//-----------------------------------------------------------------------------
static int quaternionbox_tostring(lua_State* L)
{
	LuaStack stack(L);
	Quaternion& q = stack.get_quaternionbox(1);
	stack.push_fstring("QuaternionBox (%p)", &q);
	return 1;
}

//-----------------------------------------------------------------------------
void load_quaternionbox(LuaEnvironment& env)
{
	env.load_module_function("QuaternionBox", "new",        quaternionbox_new);
	env.load_module_function("QuaternionBox", "store",      quaternionbox_store);
	env.load_module_function("QuaternionBox", "unbox",      quaternionbox_unbox);
	env.load_module_function("QuaternionBox", "__tostring", quaternionbox_tostring);

	env.load_module_constructor("QuaternionBox",			quaternionbox_ctor);
}

} // namespace crown
