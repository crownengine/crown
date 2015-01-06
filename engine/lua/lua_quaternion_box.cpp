/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "quaternion.h"
#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

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

static int quaternionbox_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return quaternionbox_new(L);
}

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

static int quaternionbox_unbox(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	stack.push_quaternion(q);
	return 1;
}

static int quaternionbox_tostring(lua_State* L)
{
	LuaStack stack(L);
	Quaternion& q = stack.get_quaternionbox(1);
	stack.push_fstring("QuaternionBox (%p)", &q);
	return 1;
}

void load_quaternionbox(LuaEnvironment& env)
{
	env.load_module_function("QuaternionBox", "new",        quaternionbox_new);
	env.load_module_function("QuaternionBox", "store",      quaternionbox_store);
	env.load_module_function("QuaternionBox", "unbox",      quaternionbox_unbox);
	env.load_module_function("QuaternionBox", "__tostring", quaternionbox_tostring);

	env.load_module_constructor("QuaternionBox", quaternionbox_ctor);
}

} // namespace crown
