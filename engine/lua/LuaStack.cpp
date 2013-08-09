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

#include "LuaStack.h"
#include "Assert.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Mat4.h"
#include "Quat.h"

namespace crown
{

static const int32_t 	LUA_VEC2_BUFFER_SIZE = 4096;
static Vec2 			vec2_buffer[LUA_VEC2_BUFFER_SIZE];
static uint32_t 		vec2_used = 0;

static const int32_t 	LUA_VEC3_BUFFER_SIZE = 4096;
static Vec3 			vec3_buffer[LUA_VEC3_BUFFER_SIZE];
static uint32_t 		vec3_used = 0;

static const int32_t 	LUA_MAT4_BUFFER_SIZE = 4096;
static Mat4 			mat4_buffer[LUA_MAT4_BUFFER_SIZE];
static uint32_t 		mat4_used = 0;

static const int32_t 	LUA_QUAT_BUFFER_SIZE = 4096;
static Quat 			quat_buffer[LUA_QUAT_BUFFER_SIZE];
static uint32_t 		quat_used = 0;

//-----------------------------------------------------------------------------
static Vec2* next_vec2(const Vec2& v)
{
	CE_ASSERT(vec2_used < LUA_VEC2_BUFFER_SIZE, "Maximum number of Vec2 reached");

	return &(vec2_buffer[vec2_used++] = v);
}

//-----------------------------------------------------------------------------
static Vec3* next_vec3(const Vec3& v)
{
	CE_ASSERT(vec3_used < LUA_VEC3_BUFFER_SIZE, "Maximum number of Vec3 reached");

	return &(vec3_buffer[vec3_used++] = v);
}

//-----------------------------------------------------------------------------
static Mat4* next_mat4(const Mat4& m)
{
	CE_ASSERT(mat4_used < LUA_MAT4_BUFFER_SIZE, "Maximum number of Mat4 reached");

	return &(mat4_buffer[mat4_used++] = m);
}

//-----------------------------------------------------------------------------
static Quat* next_quat(const Quat& q)
{
	CE_ASSERT(quat_used < LUA_QUAT_BUFFER_SIZE, "Maximum number of Quat reached");

	return &(quat_buffer[quat_used++] = q);
}

//-----------------------------------------------------------------------------	
LuaStack::LuaStack(lua_State* L)
{
	m_state = L;
}

//-----------------------------------------------------------------------------
lua_State* LuaStack::state()
{
	return m_state;
}

//-----------------------------------------------------------------------------
int32_t LuaStack::num_args()
{
	return lua_gettop(m_state);
}

//-----------------------------------------------------------------------------
void LuaStack::push_bool(bool value)
{
	lua_pushboolean(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_int32(int32_t value)
{
	lua_pushinteger(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_uint32(uint32_t value)
{
	lua_pushinteger(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_int64(int64_t value)
{
	lua_pushinteger(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_uint64(uint64_t value)
{
	lua_pushinteger(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_float(float value)
{
	lua_pushnumber(m_state, value);
}

//-----------------------------------------------------------------------------
void LuaStack::push_string(const char* s)
{
	lua_pushstring(m_state, s);
}

//-----------------------------------------------------------------------------
void LuaStack::push_literal(const char* s, size_t len)
{
	lua_pushlstring(m_state, s, len);
}

//-----------------------------------------------------------------------------
void LuaStack::push_lightdata(void* data)
{
	lua_pushlightuserdata(m_state, data);
}

//-----------------------------------------------------------------------------
void LuaStack::push_vec2(const Vec2& v)
{
	lua_pushlightuserdata(m_state, next_vec2(v));
}

//-----------------------------------------------------------------------------
void LuaStack::push_vec3(const Vec3& v)
{
	lua_pushlightuserdata(m_state, next_vec3(v));
}

//-----------------------------------------------------------------------------
void LuaStack::push_mat4(const Mat4& m)
{
	lua_pushlightuserdata(m_state, next_mat4(m));
}

//-----------------------------------------------------------------------------
void LuaStack::push_quat(const Quat& q)
{
	lua_pushlightuserdata(m_state, next_quat(q));
}

//-----------------------------------------------------------------------------
bool LuaStack::get_bool(int32_t index)
{
	return (bool) luaL_checkinteger(m_state, index);
}

//-----------------------------------------------------------------------------
int32_t LuaStack::get_int(int32_t index)
{
	return luaL_checkinteger(m_state, index);
}

//-----------------------------------------------------------------------------
float LuaStack::get_float(int32_t index)
{
	return luaL_checknumber(m_state, index);
}

//-----------------------------------------------------------------------------
const char* LuaStack::get_string(int32_t index)
{
	return luaL_checkstring(m_state, index);
}

//-----------------------------------------------------------------------------
void* LuaStack::get_lightdata(int32_t index)
{
	return lua_touserdata(m_state, index);	
}

//-----------------------------------------------------------------------------
Vec2& LuaStack::get_vec2(int32_t index)
{
	void* v = lua_touserdata(m_state, index);

	if (v < &vec2_buffer[0] || v > &vec2_buffer[LUA_VEC2_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Vec2");
	}

	return *(Vec2*)v;
}

//-----------------------------------------------------------------------------
Vec3& LuaStack::get_vec3(int32_t index)
{
	void* v = lua_touserdata(m_state, index);

	if (v < &vec3_buffer[0] || v > &vec3_buffer[LUA_VEC3_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Vec3");
	}

	return *(Vec3*)v;
}

//-----------------------------------------------------------------------------
Mat4& LuaStack::get_mat4(int32_t index)
{
	void* m = lua_touserdata(m_state, index);

	if (m < &mat4_buffer[0] || m > &mat4_buffer[LUA_MAT4_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Mat4");
	}

	return *(Mat4*)m;
}

//-----------------------------------------------------------------------------
Quat& LuaStack::get_quat(int32_t index)
{
	void* q = lua_touserdata(m_state, index);

	if (q < &quat_buffer[0] || q > &quat_buffer[LUA_QUAT_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Quat");
	}

	return *(Quat*)q;
}

} // namespace crown
