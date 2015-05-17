/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "device.h"

namespace crown
{

bool is_vector3(int i) { return device()->lua_environment()->is_vector3(i); }
bool is_quaternion(int i) { return device()->lua_environment()->is_quaternion(i); }
bool is_matrix4x4(int i) { return device()->lua_environment()->is_matrix4x4(i); }

Vector2 LuaStack::get_vector2(int i)
{
	void* v = CHECKLIGHTDATA(L, i, is_vector3, "Vector2");
	Vector3& vv = *(Vector3*)v;
	return Vector2(vv.x, vv.y);
}

Vector3& LuaStack::get_vector3(int i)
{
	void* v = CHECKLIGHTDATA(L, i, is_vector3, "Vector3");
	return *(Vector3*)v;
}

Quaternion& LuaStack::get_quaternion(int i)
{
	void* q = CHECKLIGHTDATA(L, i, is_quaternion, "Quaternion");
	return *(Quaternion*)q;
}

Matrix4x4& LuaStack::get_matrix4x4(int i)
{
	void* m = CHECKLIGHTDATA(L, i, is_matrix4x4, "Matrix4x4");
	return *(Matrix4x4*)m;
}

Color4 LuaStack::get_color4(int i)
{
	// Color4 represented as Quaternion
	void* c = CHECKLIGHTDATA(L, i, is_quaternion, "Color4");
	Quaternion& q = *(Quaternion*)c;
	return Color4(q.x, q.y, q.z, q.w);
}

void LuaStack::push_vector2(const Vector2& v)
{
	push_vector3(Vector3(v.x, v.y, 0.0f));
}

void LuaStack::push_vector3(const Vector3& v)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_vector3(v));
	luaL_getmetatable(L, "Lightuserdata_mt");
	lua_setmetatable(L, -2);
}

void LuaStack::push_quaternion(const Quaternion& q)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_quaternion(q));
	luaL_getmetatable(L, "Lightuserdata_mt");
	lua_setmetatable(L, -2);
}

void LuaStack::push_matrix4x4(const Matrix4x4& m)
{
	lua_pushlightuserdata(L, device()->lua_environment()->next_matrix4x4(m));
	luaL_getmetatable(L, "Lightuserdata_mt");
	lua_setmetatable(L, -2);
}

} // namespace crown
