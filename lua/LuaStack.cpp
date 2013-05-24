#include "LuaStack.h"
#include <cassert>

namespace crown
{

//-------------------------------------------------------	
LuaStack::LuaStack(lua_State* L)
{
	m_state = L;
}

//-------------------------------------------------------
void LuaStack::push_bool(bool value)
{
	lua_pushboolean(m_state, value);
}

//-------------------------------------------------------
void LuaStack::push_int(int32_t value)
{
	lua_pushinteger(m_state, value);
}

//-------------------------------------------------------
void LuaStack::push_float(float value)
{
	lua_pushnumber(m_state, value);
}

//-------------------------------------------------------
void LuaStack::push_string(const char* str, size_t len)
{
	lua_pushlstring(m_state, str, len);
}

//-------------------------------------------------------
void LuaStack::push_vec2(Vec2* v)
{
	assert(v >= &vec2_buffer[0] && v <= &vec2_buffer[LUA_VEC2_BUFFER_SIZE-1]);

	lua_pushlightuserdata(m_state, v);
}

void LuaStack::push_vec3(Vec3* v)
{
	assert(v >= &vec3_buffer[0] && v <= &vec3_buffer[LUA_VEC3_BUFFER_SIZE-1]);

	lua_pushlightuserdata(m_state, v);
}

void LuaStack::push_mat4(Mat4* m)
{
	assert(m >= &mat4_buffer[0] && m <= &mat4_buffer[LUA_MAT4_BUFFER_SIZE-1]);

	lua_pushlightuserdata(m_state, m);
}

void LuaStack::push_quat(Quat* q)
{
	assert(q >= &quat_buffer[0] && q <= &quat_buffer[LUA_MAT4_BUFFER_SIZE-1]);

	lua_pushlightuserdata(m_state, q);
}

//-------------------------------------------------------
bool LuaStack::get_bool(int32_t index)
{
	return (bool) luaL_checkinteger(m_state, index);
}

//-------------------------------------------------------
int32_t LuaStack::get_int(int32_t index)
{
	return luaL_checkinteger(m_state, index);
}

//-------------------------------------------------------
float LuaStack::get_float(int32_t index)
{
	return luaL_checknumber(m_state, index);
}

//-------------------------------------------------------
const char* LuaStack::get_string(int32_t index)
{
	return luaL_checkstring(m_state, index);
}

Vec2* LuaStack::get_vec2(int32_t index)
{
	assert(lua_islightuserdata(m_state, index));

	Vec2* v = (Vec2*)lua_touserdata(m_state, index);

	assert(v >= &vec2_buffer[0] && v <= &vec2_buffer[LUA_VEC2_BUFFER_SIZE-1]);

	return v;
}

//-------------------------------------------------------
Vec3* LuaStack::get_vec3(int32_t index)
{
	assert(lua_islightuserdata(m_state, index));

	Vec3* v = (Vec3*)lua_touserdata(m_state, index);

	assert(v >= &vec3_buffer[0] && v <= &vec3_buffer[LUA_VEC3_BUFFER_SIZE-1]);

	return v;
}

//-------------------------------------------------------
Mat4* LuaStack::get_mat4(int32_t index)
{
	assert(lua_islightuserdata(m_state, index));

	Mat4* m = (Mat4*)lua_touserdata(m_state, index);

	assert(m >= &mat4_buffer[0] && m <= &mat4_buffer[LUA_MAT4_BUFFER_SIZE-1]);

	return m;
}

//-------------------------------------------------------
Quat* LuaStack::get_quat(int32_t index)
{
	assert(lua_islightuserdata(m_state, index));

	Quat* q = (Quat*)lua_touserdata(m_state, index);

	assert(q >= &quat_buffer[0] && q <= &quat_buffer[LUA_QUAT_BUFFER_SIZE-1]);

	return q;
}


} // namespace crown