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

#pragma once

#include "lua.hpp"
#include "Types.h"

namespace crown
{

class Vec2;
class Vec3;
class Mat4;
class Quat;
class Unit;
class Camera;
class World;
class Mesh;

class LuaStack
{
public:

	//-----------------------------------------------------------------------------	
	LuaStack(lua_State* L)
		: m_state(L)
	{
	}

	//-----------------------------------------------------------------------------
	lua_State* state()
	{
		return m_state;
	}

	/// Returns the number of elements in the stack.
	/// When called inside a function, it can be used to count
	/// the number of arguments passed to the function itself.
	int32_t num_args()
	{
		return lua_gettop(m_state);
	}

	//-----------------------------------------------------------------------------
	void push_bool(bool value)
	{
		lua_pushboolean(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_int32(int32_t value)
	{
		lua_pushinteger(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_uint32(uint32_t value)
	{
		lua_pushinteger(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_int64(int64_t value)
	{
		lua_pushinteger(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_uint64(uint64_t value)
	{
		lua_pushinteger(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_float(float value)
	{
		lua_pushnumber(m_state, value);
	}

	//-----------------------------------------------------------------------------
	void push_string(const char* s)
	{
		lua_pushstring(m_state, s);
	}

	//-----------------------------------------------------------------------------
	void push_literal(const char* s, size_t len)
	{
		lua_pushlstring(m_state, s, len);
	}

	//-----------------------------------------------------------------------------
	void push_lightdata(void* data)
	{
		lua_pushlightuserdata(m_state, data);
	}

	//-----------------------------------------------------------------------------
	bool get_bool(int32_t index)
	{
		return (bool) lua_toboolean(m_state, index);
	}

	//-----------------------------------------------------------------------------
	int32_t get_int(int32_t index)
	{
		return luaL_checkinteger(m_state, index);
	}

	//-----------------------------------------------------------------------------
	float get_float(int32_t index)
	{
		return luaL_checknumber(m_state, index);
	}

	//-----------------------------------------------------------------------------
	const char* get_string(int32_t index)
	{
		return luaL_checkstring(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void* get_lightdata(int32_t index)
	{
		return lua_touserdata(m_state, index);	
	}

	//-----------------------------------------------------------------------------
	void push_world(World* world)
	{
		lua_pushlightuserdata(m_state, world);
	}

	//-----------------------------------------------------------------------------
	World* get_world(int32_t index)
	{
		return (World*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_unit(Unit* unit)
	{
		lua_pushlightuserdata(m_state, unit);
	}

	//-----------------------------------------------------------------------------
	Unit* get_unit(int32_t index)
	{
		return (Unit*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_camera(Camera* camera)
	{
		lua_pushlightuserdata(m_state, camera);
	}

	//-----------------------------------------------------------------------------
	Camera* get_camera(int32_t index)
	{
		return (Camera*) lua_touserdata(m_state, index);
	}

	//-----------------------------------------------------------------------------
	void push_mesh(Mesh* mesh)
	{
		lua_pushlightuserdata(m_state, mesh);
	}

	//-----------------------------------------------------------------------------
	Mesh* get_mesh(int32_t index)
	{
		return (Mesh*) lua_touserdata(m_state, index);
	}

	Vec2& get_vec2(int32_t index);
	Vec3& get_vec3(int32_t index);
	Mat4& get_mat4(int32_t index);
	Quat& get_quat(int32_t index);
	void push_vec2(const Vec2& v);
	void push_vec3(const Vec3& v);
	void push_mat4(const Mat4& m);
	void push_quat(const Quat& q);

private:

	lua_State* m_state;
};

} // namespace crown