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

static int quaternion_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion(stack.get_vector3(1), stack.get_float(2)));
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
	stack.push_quaternion(QUATERNION_IDENTITY);
	return 1;
}

static int quaternion_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(normalize(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_conjugate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(conjugate(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_inverse(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(inverse(stack.get_quaternion(1)));
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
	stack.push_quaternion(power(stack.get_quaternion(1), stack.get_float(2)));
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

static int quaternionbox_new(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 1)
		stack.push_quaternionbox(stack.get_quaternion(1));
	else
		stack.push_quaternionbox(quaternion(stack.get_float(1)
			, stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4)));

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
		q = stack.get_quaternion(2);
	else
		q = quaternion(stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4)
			, stack.get_float(5));

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

	env.load_module_function("QuaternionBox", "new",        quaternionbox_new);
	env.load_module_function("QuaternionBox", "store",      quaternionbox_store);
	env.load_module_function("QuaternionBox", "unbox",      quaternionbox_unbox);
	env.load_module_function("QuaternionBox", "__tostring", quaternionbox_tostring);

	env.load_module_constructor("QuaternionBox", quaternionbox_ctor);
}

} //namespace crown
