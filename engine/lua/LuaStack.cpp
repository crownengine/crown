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
#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"

namespace crown
{

static const uint32_t 	LUA_VEC2_BUFFER_SIZE = 4096;
static Vector2 			g_vec2_buffer[LUA_VEC2_BUFFER_SIZE];
static uint32_t 		g_vec2_used = 0;

static const uint32_t 	LUA_VEC3_BUFFER_SIZE = 4096;
static Vector3 			g_vec3_buffer[LUA_VEC3_BUFFER_SIZE];
static uint32_t 		g_vec3_used = 0;

static const uint32_t 	LUA_MAT4_BUFFER_SIZE = 4096;
static Matrix4x4 		g_mat4_buffer[LUA_MAT4_BUFFER_SIZE];
static uint32_t 		g_mat4_used = 0;

static const uint32_t 	LUA_QUAT_BUFFER_SIZE = 4096;
static Quaternion 		g_quat_buffer[LUA_QUAT_BUFFER_SIZE];
static uint32_t 		g_quat_used = 0;

//-----------------------------------------------------------------------------
static Vector2* next_vec2(const Vector2& v)
{
	CE_ASSERT(g_vec2_used < LUA_VEC2_BUFFER_SIZE, "Maximum number of Vector2 reached");

	return &(g_vec2_buffer[g_vec2_used++] = v);
}

//-----------------------------------------------------------------------------
static Vector3* next_vec3(const Vector3& v)
{
	CE_ASSERT(g_vec3_used < LUA_VEC3_BUFFER_SIZE, "Maximum number of Vector3 reached");

	return &(g_vec3_buffer[g_vec3_used++] = v);
}

//-----------------------------------------------------------------------------
static Matrix4x4* next_mat4(const Matrix4x4& m)
{
	CE_ASSERT(g_mat4_used < LUA_MAT4_BUFFER_SIZE, "Maximum number of Matrix4x4 reached");

	return &(g_mat4_buffer[g_mat4_used++] = m);
}

//-----------------------------------------------------------------------------
static Quaternion* next_quat(const Quaternion& q)
{
	CE_ASSERT(g_quat_used < LUA_QUAT_BUFFER_SIZE, "Maximum number of Quaternion reached");

	return &(g_quat_buffer[g_quat_used++] = q);
}

//-----------------------------------------------------------------------------
void clear_lua_temporaries()
{
	g_vec2_used = 0;
	g_vec3_used = 0;
	g_mat4_used = 0;
	g_quat_used = 0;
}

//-----------------------------------------------------------------------------
Vector2& LuaStack::get_vector2(int32_t index)
{
	void* v = lua_touserdata(m_state, index);

	if (v < &g_vec2_buffer[0] || v > &g_vec2_buffer[LUA_VEC2_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Vector2");
	}

	return *(Vector2*)v;
}

//-----------------------------------------------------------------------------
Vector3& LuaStack::get_vector3(int32_t index)
{
	void* v = lua_touserdata(m_state, index);

	if (v < &g_vec3_buffer[0] || v > &g_vec3_buffer[LUA_VEC3_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Vector3");
	}

	return *(Vector3*)v;
}

//-----------------------------------------------------------------------------
Matrix4x4& LuaStack::get_matrix4x4(int32_t index)
{
	void* m = lua_touserdata(m_state, index);

	if (m < &g_mat4_buffer[0] || m > &g_mat4_buffer[LUA_MAT4_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Matrix4x4");
	}

	return *(Matrix4x4*)m;
}

//-----------------------------------------------------------------------------
Quaternion& LuaStack::get_quaternion(int32_t index)
{
	void* q = lua_touserdata(m_state, index);

	if (q < &g_quat_buffer[0] || q > &g_quat_buffer[LUA_QUAT_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Quaternion");
	}

	return *(Quaternion*)q;
}

//-----------------------------------------------------------------------------
void LuaStack::push_vector2(const Vector2& v)
{
	lua_pushlightuserdata(m_state, next_vec2(v));
}

//-----------------------------------------------------------------------------
void LuaStack::push_vector3(const Vector3& v)
{
	lua_pushlightuserdata(m_state, next_vec3(v));
}

//-----------------------------------------------------------------------------
void LuaStack::push_matrix4x4(const Matrix4x4& m)
{
	lua_pushlightuserdata(m_state, next_mat4(m));
}

//-----------------------------------------------------------------------------
void LuaStack::push_quaternion(const Quaternion& q)
{
	lua_pushlightuserdata(m_state, next_quat(q));
}

} // namespace crown
