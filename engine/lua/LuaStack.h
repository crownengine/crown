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
#include "IdTable.h"

namespace crown
{

class Vector2;
class Vector3;
class Matrix4x4;
class Quaternion;
class Unit;
class Camera;
class World;
class Mesh;
class Sprite;
typedef Id SoundInstanceId;

void clear_lua_temporaries();

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

	/// Pushes an empty table onto the stack.
	/// When you want to set keys on the table, you have to use LuaStack::push_key_begin()
	/// and LuaStack::push_key_end() as in the following example:
	///
	/// LuaStack stack(L)
	/// stack.push_table()
	/// stack.push_key_begin("foo"); stack.push_foo(); stack.push_key_end()
	/// stack.push_key_begin("bar"); stack.push_bar(); stack.push_key_end()
	/// return 1;
	void push_table()
	{
		lua_newtable(m_state);
	}

	/// See Stack::push_table()
	void push_key_begin(const char* key)
	{
		lua_pushstring(m_state, key);
	}

	/// See Stack::push_table()
	void push_key_begin(int32_t i)
	{
		lua_pushnumber(m_state, i);
	}

	/// See Stack::push_table()
	void push_key_end()
	{
		lua_settable(m_state, -3);
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

	//-----------------------------------------------------------------------------
	void push_sprite(Sprite* sprite)
	{
		lua_pushlightuserdata(m_state, sprite);
	}

	//-----------------------------------------------------------------------------
	Sprite* get_sprite(int32_t index)
	{
		return (Sprite*) lua_touserdata(m_state, index);
	}

	//-------------------------------------------------------------------------
	void push_sound_instance_id(const SoundInstanceId id)
	{
		uintptr_t enc = id.encode();
		lua_pushlightuserdata(m_state, (void*)enc);
	}

	//-------------------------------------------------------------------------
	SoundInstanceId get_sound_instance_id(int32_t index)
	{
		uint32_t enc = (uintptr_t) lua_touserdata(m_state, index);
		SoundInstanceId id;
		id.decode(enc);
		return id;
	}

	bool is_vector2(int32_t index);
	bool is_vector3(int32_t index);
	bool is_matrix4x4(int32_t index);
	bool is_quaternion(int32_t index);
	Vector2& get_vector2(int32_t index);
	Vector3& get_vector3(int32_t index);
	Matrix4x4& get_matrix4x4(int32_t index);
	Quaternion& get_quaternion(int32_t index);
	void push_vector2(const Vector2& v);
	void push_vector3(const Vector3& v);
	void push_matrix4x4(const Matrix4x4& m);
	void push_quaternion(const Quaternion& q);

private:

	lua_State* m_state;
};

} // namespace crown