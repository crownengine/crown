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
#include "LuaSystem.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "StringUtils.h"
#include "Color4.h"

//-----------------------------------------------------------------------------
#if defined(CROWN_DEBUG)
	static void* checkudata(lua_State* L, int index, const char* expected)
	{
		luaL_checktype(L, index, LUA_TUSERDATA);
		return luaL_checkudata(L, index, expected);
	}

	typedef bool (*checkfn)(int);
	static void* checklightdata(lua_State* L, int index, checkfn cf, const char* expected)
	{
		luaL_checktype(L, index, LUA_TLIGHTUSERDATA);
		if (!cf(index)) luaL_typerror(L, index, expected);
		return lua_touserdata(L, index);
	}

	static bool always_true(int index)
	{
		return index == index;
	}

	#define CHECKUDATA(stack, index, expected) checkudata(stack, index, expected)
	#define CHECKLIGHTDATA(stack, index, cf, expected) checklightdata(stack, index, cf, expected)
	#define CHECKBOOLEAN(stack, index) lua_toboolean(stack, index)
	#define CHECKINTEGER(stack, index) luaL_checkinteger(stack, index)
	#define CHECKNUMBER(stack, index) luaL_checknumber(stack, index)
	#define CHECKSTRING(stack, index) luaL_checkstring(stack, index)
#else
	#define CHECKUDATA(stack, index, expected) lua_touserdata(stack, index)
	#define CHECKLIGHTDATA(stack, index, cf, expected) lua_touserdata(stack, index)
	#define CHECKBOOLEAN(stack, index) lua_toboolean(stack, index)
	#define CHECKINTEGER(stack, index) lua_tointeger(stack, index)
	#define CHECKNUMBER(stack, index) lua_tonumber(stack, index)
	#define CHECKSTRING(stack, index) lua_tostring(stack, index)
#endif

namespace crown
{

class PhysicsWorld;
class SoundWorld;
class World;
struct Actor;
struct Camera;
struct Controller;
struct Gui;
struct Mesh;
struct ResourcePackage;
struct Sprite;
struct Unit;
struct DebugLine;
struct Raycast;

typedef Id SoundInstanceId;
typedef Id GuiId;
typedef Id GuiComponentId;

typedef int (*MetamethodFunction)(lua_State*);

class LuaStack
{
public:

	//-----------------------------------------------------------------------------	
	LuaStack(lua_State* L)
		: m_L(L)
	{
	}

	//-----------------------------------------------------------------------------
	lua_State* state()
	{
		return m_L;
	}

	/// Returns the number of elements in the stack.
	/// When called inside a function, it can be used to count
	/// the number of arguments passed to the function itself.
	int32_t num_args()
	{
		return lua_gettop(m_L);
	}

	/// Removes the element at the given valid index, shifting down the elements
	/// above this index to fill the gap. Cannot be called with a pseudo-index,
	/// because a pseudo-index is not an actual stack position.
	void remove(int32_t index)
	{
		lua_remove(m_L, index);
	}

	//-----------------------------------------------------------------------------
	bool is_nil(int32_t index)
	{
		return lua_isnil(m_L, index);
	}

	//-----------------------------------------------------------------------------
	bool is_number(int32_t index)
	{
		return (bool) lua_isnumber(m_L, index);
	}

	/// Wraps lua_type.
	int value_type(uint32_t index)
	{
		return lua_type(m_L, index);
	}

	//-----------------------------------------------------------------------------
	void push_nil()
	{
		lua_pushnil(m_L);
	}

	//-----------------------------------------------------------------------------
	void push_bool(bool value)
	{
		lua_pushboolean(m_L, value);
	}

	//-----------------------------------------------------------------------------
	void push_int32(int32_t value)
	{
		lua_pushinteger(m_L, value);
	}

	//-----------------------------------------------------------------------------
	void push_uint32(uint32_t value)
	{
		lua_pushinteger(m_L, value);
	}

	//-----------------------------------------------------------------------------
	void push_int64(int64_t value)
	{
		lua_pushinteger(m_L, value);
	}

	//-----------------------------------------------------------------------------
	void push_uint64(uint64_t value)
	{
		lua_pushinteger(m_L, value);
	}

	//-----------------------------------------------------------------------------
	void push_float(float value)
	{
		lua_pushnumber(m_L, value);
	}

	//-----------------------------------------------------------------------------
	void push_string(const char* s)
	{
		lua_pushstring(m_L, s);
	}

	//-----------------------------------------------------------------------------
	void push_fstring(const char* fmt, ...)
	{
		va_list vl;
		va_start(vl, fmt);
		lua_pushvfstring(m_L, fmt, vl);
		va_end(vl);
	}

	//-----------------------------------------------------------------------------
	void push_literal(const char* s, size_t len)
	{
		lua_pushlstring(m_L, s, len);
	}

	//-----------------------------------------------------------------------------
	bool get_bool(int32_t index)
	{
		return (bool) CHECKBOOLEAN(m_L, index);
	}

	//-----------------------------------------------------------------------------
	int32_t get_int(int32_t index)
	{
		return CHECKINTEGER(m_L, index);
	}

	//-----------------------------------------------------------------------------
	float get_float(int32_t index)
	{
		return CHECKNUMBER(m_L, index);
	}

	//-----------------------------------------------------------------------------
	const char* get_string(int32_t index)
	{
		return CHECKSTRING(m_L, index);
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
		lua_newtable(m_L);
	}

	/// See Stack::push_table()
	void push_key_begin(const char* key)
	{
		lua_pushstring(m_L, key);
	}

	/// See Stack::push_table()
	void push_key_begin(int32_t i)
	{
		lua_pushnumber(m_L, i);
	}

	/// See Stack::push_table()
	void push_key_end()
	{
		lua_settable(m_L, -3);
	}

	//-----------------------------------------------------------------------------
	void push_resource_package(ResourcePackage* package)
	{
		ResourcePackage** p = (ResourcePackage**) lua_newuserdata(m_L, sizeof(ResourcePackage*));
		*p = package;
		luaL_getmetatable(m_L, "ResourcePackage");
		lua_setmetatable(m_L, -2);
	}

	//-----------------------------------------------------------------------------
	ResourcePackage* get_resource_package(int32_t index)
	{
		ResourcePackage* pkg = *(ResourcePackage**) CHECKUDATA(m_L, index, "ResourcePackage");
		return pkg;
	}

	//-----------------------------------------------------------------------------
	void push_world(World* world)
	{
		World** w = (World**) lua_newuserdata(m_L, sizeof(World*));
		*w = world;
		luaL_getmetatable(m_L, "World");
		lua_setmetatable(m_L, -2);
	};

	//-----------------------------------------------------------------------------
	World* get_world(int32_t index)
	{
		World* w = *(World**) CHECKUDATA(m_L, index, "World");
		return w;
	};

	//-----------------------------------------------------------------------------
	void push_physics_world(PhysicsWorld* world)
	{
		PhysicsWorld** w = (PhysicsWorld**) lua_newuserdata(m_L, sizeof(PhysicsWorld*));
		luaL_getmetatable(m_L, "PhysicsWorld");
		lua_setmetatable(m_L, -2);
		*w = world;
	}

	//-----------------------------------------------------------------------------
	PhysicsWorld* get_physics_world(int32_t index)
	{
		PhysicsWorld* w = *(PhysicsWorld**) CHECKUDATA(m_L, index, "PhysicsWorld");
		return w;
	}

	//-----------------------------------------------------------------------------
	void push_sound_world(SoundWorld* world)
	{
		SoundWorld** w = (SoundWorld**) lua_newuserdata(m_L, sizeof(SoundWorld*));
		*w = world;
		luaL_getmetatable(m_L, "SoundWorld");
		lua_setmetatable(m_L, -2);
	}

	//-----------------------------------------------------------------------------
	SoundWorld* get_sound_world(int32_t index)
	{
		SoundWorld* w = *(SoundWorld**) CHECKUDATA(m_L, index, "SoundWorld");
		return w;
	}

	//-----------------------------------------------------------------------------
	void push_unit(Unit* unit)
	{
		lua_pushlightuserdata(m_L, unit);
	}

	//-----------------------------------------------------------------------------
	Unit* get_unit(int32_t index)
	{
		return (Unit*) CHECKLIGHTDATA(m_L, index, always_true, "Unit");
	}

	//-----------------------------------------------------------------------------
	void push_camera(Camera* camera)
	{
		lua_pushlightuserdata(m_L, camera);
	}

	//-----------------------------------------------------------------------------
	Camera* get_camera(int32_t index)
	{
		return (Camera*) CHECKLIGHTDATA(m_L, index, always_true, "Camera");
	}

	//-----------------------------------------------------------------------------
	void push_mesh(Mesh* mesh)
	{
		lua_pushlightuserdata(m_L, mesh);
	}

	//-----------------------------------------------------------------------------
	Mesh* get_mesh(int32_t index)
	{
		return (Mesh*) CHECKLIGHTDATA(m_L, index, always_true, "Mesh");
	}

	//-----------------------------------------------------------------------------
	void push_sprite(Sprite* sprite)
	{
		lua_pushlightuserdata(m_L, sprite);
	}

	//-----------------------------------------------------------------------------
	Sprite* get_sprite(int32_t index)
	{
		return (Sprite*) CHECKLIGHTDATA(m_L, index, always_true, "Sprite");
	}

	//-----------------------------------------------------------------------------
	void push_actor(Actor* actor)
	{
		lua_pushlightuserdata(m_L, actor);
	}

	//-----------------------------------------------------------------------------
	Actor* get_actor(int32_t index)
	{
		return (Actor*) CHECKLIGHTDATA(m_L, index, always_true, "Actor");
	}

	//-----------------------------------------------------------------------------
	void push_controller(Controller* controller)
	{
		lua_pushlightuserdata(m_L, controller);
	}

	//-----------------------------------------------------------------------------
	Controller* get_controller(int32_t index)
	{
		return (Controller*) CHECKLIGHTDATA(m_L, index, always_true, "Controller");
	}

	//-----------------------------------------------------------------------------
	void push_raycast(Raycast* raycast)
	{
		lua_pushlightuserdata(m_L, raycast);
	}

	//-----------------------------------------------------------------------------
	Raycast* get_raycast(int32_t index)
	{
		return (Raycast*) CHECKLIGHTDATA(m_L, index, always_true, "Raycast");
	}

	//-----------------------------------------------------------------------------
	void push_sound_instance_id(const SoundInstanceId id)
	{
		uintptr_t enc = id.encode();
		lua_pushlightuserdata(m_L, (void*)enc);
	}

	//-----------------------------------------------------------------------------
	SoundInstanceId get_sound_instance_id(int32_t index)
	{
		uint32_t enc = (uintptr_t) CHECKLIGHTDATA(m_L, index, always_true, "SoundInstanceId");
		SoundInstanceId id;
		id.decode(enc);
		return id;
	}

	//-----------------------------------------------------------------------------
	void push_gui(Gui* gui)
	{
		lua_pushlightuserdata(m_L, gui);
	}

	//-----------------------------------------------------------------------------
	Gui* get_gui(int32_t index)
	{
		return (Gui*) CHECKLIGHTDATA(m_L, index, always_true, "Gui");
	}

	//-----------------------------------------------------------------------------
	void push_debug_line(DebugLine* line)
	{
		DebugLine** l = (DebugLine**) lua_newuserdata(m_L, sizeof(DebugLine*));
		*l = line;
		luaL_getmetatable(m_L, "DebugLine");
		lua_setmetatable(m_L, -2);
	}

	//-----------------------------------------------------------------------------
	DebugLine* get_debug_line(int32_t index)
	{
		DebugLine* l = *(DebugLine**) CHECKUDATA(m_L, index, "DebugLine");
		return l;
	}

	//-----------------------------------------------------------------------------
	Vector2& get_vector2(int32_t index)
	{
		void* v = CHECKLIGHTDATA(m_L, index, lua_system::is_vector2, "Vector2");
		return *(Vector2*)v;
	}

	//-----------------------------------------------------------------------------
	Vector3& get_vector3(int32_t index)
	{
		void* v = CHECKLIGHTDATA(m_L, index, lua_system::is_vector3, "Vector3");
		return *(Vector3*)v;
	}

	//-----------------------------------------------------------------------------
	Matrix4x4& get_matrix4x4(int32_t index)
	{
		void* m = CHECKLIGHTDATA(m_L, index, lua_system::is_matrix4x4, "Matrix4x4");
		return *(Matrix4x4*)m;
	}

	//-----------------------------------------------------------------------------
	Quaternion& get_quaternion(int32_t index)
	{
		void* q = CHECKLIGHTDATA(m_L, index, lua_system::is_quaternion, "Quaternion");
		return *(Quaternion*)q;
	}

	Color4 get_color4(int32_t index)
	{
		// Color4 represented as Quaternion
		void* c = CHECKLIGHTDATA(m_L, index, lua_system::is_quaternion, "Color4");
		Quaternion& q = *(Quaternion*)c;
		return Color4(q.x, q.y, q.z, q.w);
	}

	//-----------------------------------------------------------------------------
	void push_vector2(const Vector2& v)
	{
		lua_pushlightuserdata(m_L, lua_system::next_vector2(v));
		luaL_getmetatable(m_L, "Lightuserdata_mt");
		lua_setmetatable(m_L, -2);
	}

	//-----------------------------------------------------------------------------
	void push_vector3(const Vector3& v)
	{
		lua_pushlightuserdata(m_L, lua_system::next_vector3(v));
		luaL_getmetatable(m_L, "Lightuserdata_mt");
		lua_setmetatable(m_L, -2);
	}

	//-----------------------------------------------------------------------------
	void push_matrix4x4(const Matrix4x4& m)
	{
		lua_pushlightuserdata(m_L, lua_system::next_matrix4x4(m));
		luaL_getmetatable(m_L, "Lightuserdata_mt");
		lua_setmetatable(m_L, -2);
	}

	//-----------------------------------------------------------------------------
	void push_quaternion(const Quaternion& q)
	{
		lua_pushlightuserdata(m_L, lua_system::next_quaternion(q));
		luaL_getmetatable(m_L, "Lightuserdata_mt");
		lua_setmetatable(m_L, -2);
	}

	//-----------------------------------------------------------------------------
	void push_vector2box(const Vector2& v)
	{
		Vector2* vec = (Vector2*) lua_newuserdata(m_L, sizeof(Vector2));
		luaL_getmetatable(m_L, "Vector2Box");
		lua_setmetatable(m_L, -2);
		*vec = v;
	}

	//-----------------------------------------------------------------------------
	Vector2& get_vector2box(uint32_t index)
	{
		Vector2* v = (Vector2*) CHECKUDATA(m_L, index, "Vector2Box");
		return *v;
	}

	//-----------------------------------------------------------------------------
	void push_vector3box(const Vector3& v)
	{
		Vector3* vec = (Vector3*) lua_newuserdata(m_L, sizeof(Vector3));
		luaL_getmetatable(m_L, "Vector3Box");
		lua_setmetatable(m_L, -2);
		*vec = v;
	}

	//-----------------------------------------------------------------------------
	Vector3& get_vector3box(uint32_t index)
	{
		Vector3* v = (Vector3*) CHECKUDATA(m_L, index, "Vector3Box");
		return *v;
	}

	//-----------------------------------------------------------------------------
	void push_quaternionbox(const Quaternion& q)
	{
		Quaternion* quat = (Quaternion*) lua_newuserdata(m_L, sizeof(Quaternion));
		luaL_getmetatable(m_L, "QuaternionBox");
		lua_setmetatable(m_L, -2);
		*quat = q;
	}

	//-----------------------------------------------------------------------------
	Quaternion& get_quaternionbox(uint32_t index)
	{
		Quaternion* q = (Quaternion*) CHECKUDATA(m_L, index, "QuaternionBox");
		return *q;
	}

	//-----------------------------------------------------------------------------
	void push_matrix4x4box(const Matrix4x4& m)
	{
		Matrix4x4* mat = (Matrix4x4*) lua_newuserdata(m_L, sizeof(Matrix4x4));
		luaL_getmetatable(m_L, "Matrix4x4Box");
		lua_setmetatable(m_L, -2);
		*mat = m;
	}

	//-----------------------------------------------------------------------------
	Matrix4x4& get_matrix4x4box(uint32_t index)
	{
		Matrix4x4* m = (Matrix4x4*) CHECKUDATA(m_L, index, "Matrix4x4Box");
		return *m;
	}

private:

	lua_State* m_L;
};

} // namespace crown