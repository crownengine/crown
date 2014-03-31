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

#include "LuaEnvironment.h"
#include "LuaStack.h"
#include "Vector3.h"
#include "StringUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int vector3box_get_value(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);
	const char* s = stack.get_string(2);

	if (string::strcmp(s, "x") == 0)
	{
		stack.push_float(v.x);
		return 1;
	}
	else if (string::strcmp(s, "y") == 0)
	{
		stack.push_float(v.y);
		return 1;
	}
	else if (string::strcmp(s, "z") == 0)
	{
		stack.push_float(v.z);
		return 1;
	}

	// Never happens
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3box_set_value(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);
	const char* s = stack.get_string(2);
	float value = stack.get_float(3);

	if (string::strcmp(s, "x") == 0)
	{
		v.x = value;
	}
	else if (string::strcmp(s, "y") == 0)
	{
		v.y = value;
	}
	else if (string::strcmp(s, "z") == 0)
	{
		v.z = value;
	}

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3box(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.push_vector3box();

	// Associates a metatable to userdata
	stack.get_global_metatable("Vector3Box_i_mt");
	stack.set_metatable();

	if (stack.num_args() == 2)
	{
		Vector3 tv = stack.get_vector3(1);

		v = tv;
	}
	else if (stack.num_args() == 4)
	{
		v.x = stack.get_float(1);
		v.y = stack.get_float(2);
		v.z = stack.get_float(3);
	}
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3box_store(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);
	
	if (stack.num_args() == 2)
	{
		Vector3 tv = stack.get_vector3(1);

		v = tv;
	}
	else if (stack.num_args() == 4)
	{
		v.x = stack.get_float(1);
		v.y = stack.get_float(2);
		v.z = stack.get_float(3);
	}
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3box_unbox(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);

	stack.push_vector3(v);
	return 1;
}

//-----------------------------------------------------------------------------
void load_vector3box(LuaEnvironment& env)
{
	env.load_module_function("Vector3Box", "new", vector3box);
	env.load_module_function("Vector3Box", "store", vector3box_store);
	env.load_module_function("Vector3Box", "unbox", vector3box_unbox);
}

} // namespace crown