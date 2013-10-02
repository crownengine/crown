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

static const uint32_t 	LUA_VEC2_BUFFER_SIZE = 4096;
static Vec2 			g_vec2_buffer[LUA_VEC2_BUFFER_SIZE];
static uint32_t 		g_vec2_used = 0;

static const uint32_t 	LUA_VEC3_BUFFER_SIZE = 4096;
static Vec3 			g_vec3_buffer[LUA_VEC3_BUFFER_SIZE];
static uint32_t 		g_vec3_used = 0;

static const uint32_t 	LUA_MAT4_BUFFER_SIZE = 4096;
static Mat4 			g_mat4_buffer[LUA_MAT4_BUFFER_SIZE];
static uint32_t 		g_mat4_used = 0;

static const uint32_t 	LUA_QUAT_BUFFER_SIZE = 4096;
static Quat 			g_quat_buffer[LUA_QUAT_BUFFER_SIZE];
static uint32_t 		g_quat_used = 0;

//-----------------------------------------------------------------------------
static Vec2* next_vec2(const Vec2& v)
{
	CE_ASSERT(g_vec2_used < LUA_VEC2_BUFFER_SIZE, "Maximum number of Vec2 reached");

	return &(g_vec2_buffer[g_vec2_used++] = v);
}

//-----------------------------------------------------------------------------
static Vec3* next_vec3(const Vec3& v)
{
	CE_ASSERT(g_vec3_used < LUA_VEC3_BUFFER_SIZE, "Maximum number of Vec3 reached");

	return &(g_vec3_buffer[g_vec3_used++] = v);
}

//-----------------------------------------------------------------------------
static Mat4* next_mat4(const Mat4& m)
{
	CE_ASSERT(g_mat4_used < LUA_MAT4_BUFFER_SIZE, "Maximum number of Mat4 reached");

	return &(g_mat4_buffer[g_mat4_used++] = m);
}

//-----------------------------------------------------------------------------
static Quat* next_quat(const Quat& q)
{
	CE_ASSERT(g_quat_used < LUA_QUAT_BUFFER_SIZE, "Maximum number of Quat reached");

	return &(g_quat_buffer[g_quat_used++] = q);
}

//-----------------------------------------------------------------------------
Vec2& LuaStack::get_vec2(int32_t index)
{
	void* v = lua_touserdata(m_state, index);

	if (v < &g_vec2_buffer[0] || v > &g_vec2_buffer[LUA_VEC2_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Vec2");
	}

	return *(Vec2*)v;
}

//-----------------------------------------------------------------------------
Vec3& LuaStack::get_vec3(int32_t index)
{
	void* v = lua_touserdata(m_state, index);

	if (v < &g_vec3_buffer[0] || v > &g_vec3_buffer[LUA_VEC3_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Vec3");
	}

	return *(Vec3*)v;
}

//-----------------------------------------------------------------------------
Mat4& LuaStack::get_mat4(int32_t index)
{
	void* m = lua_touserdata(m_state, index);

	if (m < &g_mat4_buffer[0] || m > &g_mat4_buffer[LUA_MAT4_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Mat4");
	}

	return *(Mat4*)m;
}

//-----------------------------------------------------------------------------
Quat& LuaStack::get_quat(int32_t index)
{
	void* q = lua_touserdata(m_state, index);

	if (q < &g_quat_buffer[0] || q > &g_quat_buffer[LUA_QUAT_BUFFER_SIZE-1])
	{
		luaL_typerror(m_state, index, "Quat");
	}

	return *(Quat*)q;
}

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

} // namespace crown
