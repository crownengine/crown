/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "device/device.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.inl"

namespace crown
{
bool LuaStack::is_vector3(int i)
{
	return device()->_lua_environment->is_vector3((Vector3*)lua_touserdata(L, i));
}

bool LuaStack::is_quaternion(int i)
{
	return device()->_lua_environment->is_quaternion((Quaternion*)lua_touserdata(L, i));
}

bool LuaStack::is_matrix4x4(int i)
{
	return device()->_lua_environment->is_matrix4x4((Matrix4x4*)lua_touserdata(L, i));
}

#if CROWN_DEBUG
Vector3* LuaStack::check_temporary(int i, const Vector3* ptr)
{
	LuaEnvironment* env = device()->_lua_environment;
	if (CE_UNLIKELY(!env->is_vector3(ptr)))
	{
		luaL_typerror(L, i, "Vector3");
		CE_UNREACHABLE();
	}

	return env->check_valid(ptr);
}

Quaternion* LuaStack::check_temporary(int i, const Quaternion* ptr)
{
	LuaEnvironment* env = device()->_lua_environment;
	if (CE_UNLIKELY(!env->is_quaternion(ptr)))
	{
		luaL_typerror(L, i, "Quaternion");
		CE_UNREACHABLE();
	}

	return env->check_valid(ptr);
}

Matrix4x4* LuaStack::check_temporary(int i, const Matrix4x4* ptr)
{
	LuaEnvironment* env = device()->_lua_environment;
	if (CE_UNLIKELY(!env->is_matrix4x4(ptr)))
	{
		luaL_typerror(L, i, "Matrix4x4");
		CE_UNREACHABLE();
	}

	return env->check_valid(ptr);
}
#endif // CROWN_DEBUG

void LuaStack::push_vector3(const Vector3& v)
{
	lua_pushlightuserdata(L, device()->_lua_environment->next_vector3(v));
}

void LuaStack::push_quaternion(const Quaternion& q)
{
	lua_pushlightuserdata(L, device()->_lua_environment->next_quaternion(q));
}

void LuaStack::push_matrix4x4(const Matrix4x4& m)
{
	lua_pushlightuserdata(L, device()->_lua_environment->next_matrix4x4(m));
}

} // namespace crown
