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

#include "lua_environment.h"
#include "lua_stack.h"
#include "vector3.h"
#include "string_utils.h"

namespace crown
{

static int vector3box_new(lua_State* L)
{
	LuaStack stack(L);

	Vector3 v;
	if (stack.num_args() == 1)
	{
		const Vector3 tv = stack.get_vector3(1);
		v = tv;
	}
	else if (stack.num_args() == 3)
	{
		v.x = stack.get_float(1);
		v.y = stack.get_float(2);
		v.z = stack.get_float(3);
	}

	stack.push_vector3box(v);
	return 1;
}

static int vector3box_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector3box_new(L);
}

static int vector3box_get_value(lua_State* L)
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

static int vector3box_set_value(lua_State* L)
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

static int vector3box_store(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);

	if (stack.num_args() == 2)
	{
		Vector3 tv = stack.get_vector3(2);

		v = tv;
	}
	else if (stack.num_args() == 4)
	{
		v.x = stack.get_float(2);
		v.y = stack.get_float(3);
		v.z = stack.get_float(4);
	}
	return 0;
}

static int vector3box_unbox(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);

	stack.push_vector3(v);
	return 1;
}

static int vector3box_tostring(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3box(1);
	stack.push_fstring("Vector3Box (%p)", &v);
	return 1;
}

void load_vector3box(LuaEnvironment& env)
{
	env.load_module_function("Vector3Box", "new",        vector3box_new);
	env.load_module_function("Vector3Box", "store",      vector3box_store);
	env.load_module_function("Vector3Box", "unbox",      vector3box_unbox);
	env.load_module_function("Vector3Box", "__index",    vector3box_get_value);
	env.load_module_function("Vector3Box", "__newindex", vector3box_set_value);
	env.load_module_function("Vector3Box", "__tostring", vector3box_tostring);
	
	env.load_module_constructor("Vector3Box", vector3box_ctor);
}

} // namespace crown
