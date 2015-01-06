/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_environment.h"
#include "lua_stack.h"
#include "lua_assert.h"
#include "vector3.h"

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

	switch (s[0])
	{
		case 'x': stack.push_float(v.x); return 1;
		case 'y': stack.push_float(v.y); return 1;
		case 'z': stack.push_float(v.z); return 1;
		default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
	}

	return 0;
}

static int vector3box_set_value(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);
	const char* s = stack.get_string(2);
	const float value = stack.get_float(3);

	switch (s[0])
	{
		case 'x': v.x = value; break;
		case 'y': v.y = value; break;
		case 'z': v.z = value; break;
		default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
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
