/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/strings/string_id.inl"
#include "device/device.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.inl"

namespace crown
{
void LuaStack::push_args(const ArgType::Enum *arg_types, const Arg *args, u32 num)
{
	for (u32 i = 0; i < num; ++i) {
		switch (arg_types[i]) {
		case ArgType::NIL: push_nil(); break;
		case ArgType::INT: push_int(args[i].int_value); break;
		case ArgType::BOOL: push_bool(args[i].bool_value); break;
		case ArgType::FLOAT: push_float(args[i].float_value); break;
		case ArgType::STRING: push_string(args[i].string_value); break;
		case ArgType::STRING_ID: push_string_id(StringId32(args[i].string_id_value)); break;
		case ArgType::POINTER: push_pointer((void *)args[i].pointer_value); break;
		case ArgType::FUNCTION: push_function(args[i].cfunction_value); break;
		case ArgType::UNIT: push_unit(args[i].unit_value); break;
		case ArgType::ID: push_id(args[i].id_value); break;
		case ArgType::VECTOR3: push_vector3(args[i].vector3_value); break;
		case ArgType::QUATERNION: push_quaternion(args[i].quaternion_value); break;
		case ArgType::MATRIX4X4: push_matrix4x4(args[i].matrix4x4_value); break;
		default: CE_FATAL("Unknown argument type"); break;
		}
	}
}

bool LuaStack::is_vector3(int i)
{
	return device()->_lua_environment->is_vector3((Vector3 *)lua_touserdata(L, i));
}

bool LuaStack::is_quaternion(int i)
{
	return device()->_lua_environment->is_quaternion((Quaternion *)lua_touserdata(L, i));
}

bool LuaStack::is_matrix4x4(int i)
{
	return device()->_lua_environment->is_matrix4x4((Matrix4x4 *)lua_touserdata(L, i));
}

#if CROWN_DEBUG
Vector3 *LuaStack::check_temporary(int i, const Vector3 *ptr)
{
	LuaEnvironment *env = device()->_lua_environment;

	Vector3 *v = env->check_valid(ptr);
	if (CE_UNLIKELY(!env->is_vector3(v))) {
		luaL_typerror(L, i, "Vector3");
		CE_UNREACHABLE();
	}

	return v;
}

Quaternion *LuaStack::check_temporary(int i, const Quaternion *ptr)
{
	LuaEnvironment *env = device()->_lua_environment;

	Quaternion *q = env->check_valid(ptr);
	if (CE_UNLIKELY(!env->is_quaternion(q))) {
		luaL_typerror(L, i, "Quaternion");
		CE_UNREACHABLE();
	}

	return q;
}

Matrix4x4 *LuaStack::check_temporary(int i, const Matrix4x4 *ptr)
{
	LuaEnvironment *env = device()->_lua_environment;

	Matrix4x4 *m = env->check_valid(ptr);
	if (CE_UNLIKELY(!env->is_matrix4x4(m))) {
		luaL_typerror(L, i, "Matrix4x4");
		CE_UNREACHABLE();
	}

	return m;
}
#endif // if CROWN_DEBUG

void LuaStack::push_vector3(const Vector3 &v)
{
	lua_pushlightuserdata(L, device()->_lua_environment->next_vector3(v));
}

void LuaStack::push_quaternion(const Quaternion &q)
{
	lua_pushlightuserdata(L, device()->_lua_environment->next_quaternion(q));
}

void LuaStack::push_matrix4x4(const Matrix4x4 &m)
{
	lua_pushlightuserdata(L, device()->_lua_environment->next_matrix4x4(m));
}

} // namespace crown
