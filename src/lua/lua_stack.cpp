/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "device.h"

namespace crown
{

bool LuaStack::is_vector3(int i)
{
	return device()->lua_environment()->is_vector3((Vector3*)get_pointer(i));
}

bool LuaStack::is_quaternion(int i)
{
	return device()->lua_environment()->is_quaternion((Quaternion*)get_pointer(i));
}

bool LuaStack::is_matrix4x4(int i)
{
	return device()->lua_environment()->is_matrix4x4((Matrix4x4*)get_pointer(i));
}

void LuaStack::check_temporary(int i, Vector3* p)
{
	LuaEnvironment* env = device()->lua_environment();
	if (!is_pointer(i) || !env->is_vector3(p))
		luaL_typerror(L, i, "Vector3");
}

void LuaStack::check_temporary(int i, Quaternion* p)
{
	LuaEnvironment* env = device()->lua_environment();
	if (!is_pointer(i) || !env->is_quaternion(p))
		luaL_typerror(L, i, "Quaternion");
}

void LuaStack::check_temporary(int i, Matrix4x4* p)
{
	LuaEnvironment* env = device()->lua_environment();
	if (!is_pointer(i) || !env->is_matrix4x4(p))
		luaL_typerror(L, i, "Matrix4x4");
}

Vector2 LuaStack::get_vector2(int i)
{
	Vector3 v = get_vector3(i);
	return vector2(v.x, v.y);
}

Vector3& LuaStack::get_vector3(int i)
{
	Vector3* v = (Vector3*)get_pointer(i);
	check_temporary(i, v);
	return *v;
}

Quaternion& LuaStack::get_quaternion(int i)
{
	Quaternion* q = (Quaternion*)get_pointer(i);
	check_temporary(i, q);
	return *q;
}

Matrix4x4& LuaStack::get_matrix4x4(int i)
{
	Matrix4x4* m = (Matrix4x4*)get_pointer(i);
	check_temporary(i, m);
	return *m;
}

Color4 LuaStack::get_color4(int i)
{
	Quaternion q = get_quaternion(i);
	return color4(q.x, q.y, q.z, q.w);
}

void LuaStack::push_vector2(const Vector2& v)
{
	push_vector3(vector3(v.x, v.y, 0.0f));
}

void LuaStack::push_vector3(const Vector3& v)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_vector3(v));
}

void LuaStack::push_quaternion(const Quaternion& q)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_quaternion(q));
}

void LuaStack::push_matrix4x4(const Matrix4x4& m)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_matrix4x4(m));
}

} // namespace crown
