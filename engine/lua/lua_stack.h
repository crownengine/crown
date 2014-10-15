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
#include "types.h"
#include "lua_system.h"
#include "vector3.h"
#include "vector2.h"
#include "quaternion.h"
#include "matrix4x4.h"
#include "string_utils.h"
#include "color4.h"

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
struct Material;

typedef Id SoundInstanceId;
typedef Id GuiId;

typedef int (*MetamethodFunction)(lua_State*);

struct LuaStack
{
	LuaStack(lua_State* L)
		: _L(L)
	{
	}

	lua_State* state()
	{
		return _L;
	}

	/// Returns the number of elements in the stack.
	/// When called inside a function, it can be used to count
	/// the number of arguments passed to the function itself.
	int32_t num_args()
	{
		return lua_gettop(_L);
	}

	/// Removes the element at the given valid index, shifting down the elements
	/// above this index to fill the gap. Cannot be called with a pseudo-index,
	/// because a pseudo-index is not an actual stack position.
	void remove(int32_t index)
	{
		lua_remove(_L, index);
	}

	/// Pops @a n elements from the stack.
	void pop(int32_t n)
	{
		lua_pop(_L, n);
	}
	bool is_nil(int32_t index)
	{
		return lua_isnil(_L, index) == 1;
	}

	bool is_number(int32_t index)
	{
		return lua_isnumber(_L, index) == 1;
	}

	/// Wraps lua_type.
	int value_type(uint32_t index)
	{
		return lua_type(_L, index);
	}

	void push_nil()
	{
		lua_pushnil(_L);
	}

	void push_bool(bool value)
	{
		lua_pushboolean(_L, value);
	}

	void push_int32(int32_t value)
	{
		lua_pushinteger(_L, value);
	}

	void push_uint32(uint32_t value)
	{
		lua_pushinteger(_L, value);
	}

	void push_float(float value)
	{
		lua_pushnumber(_L, value);
	}

	void push_string(const char* s)
	{
		lua_pushstring(_L, s);
	}

	void push_fstring(const char* fmt, ...)
	{
		va_list vl;
		va_start(vl, fmt);
		lua_pushvfstring(_L, fmt, vl);
		va_end(vl);
	}

	void push_literal(const char* s, size_t len)
	{
		lua_pushlstring(_L, s, len);
	}

	bool get_bool(int32_t index)
	{
		return CHECKBOOLEAN(_L, index) == 1;
	}

	int32_t get_int(int32_t index)
	{
		return CHECKINTEGER(_L, index);
	}

	float get_float(int32_t index)
	{
		return (float) CHECKNUMBER(_L, index);
	}

	const char* get_string(int32_t index)
	{
		return CHECKSTRING(_L, index);
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
		lua_newtable(_L);
	}

	/// See Stack::push_table()
	void push_key_begin(const char* key)
	{
		lua_pushstring(_L, key);
	}

	/// See Stack::push_table()
	void push_key_begin(int32_t i)
	{
		lua_pushnumber(_L, i);
	}

	/// See Stack::push_table()
	void push_key_end()
	{
		lua_settable(_L, -3);
	}

	int next(int32_t i)
	{
		return lua_next(_L, i);
	}

	void push_resource_package(ResourcePackage* package)
	{
		ResourcePackage** p = (ResourcePackage**) lua_newuserdata(_L, sizeof(ResourcePackage*));
		*p = package;
		luaL_getmetatable(_L, "ResourcePackage");
		lua_setmetatable(_L, -2);
	}

	ResourcePackage* get_resource_package(int32_t index)
	{
		ResourcePackage* pkg = *(ResourcePackage**) CHECKUDATA(_L, index, "ResourcePackage");
		return pkg;
	}

	void push_world(World* world)
	{
		World** w = (World**) lua_newuserdata(_L, sizeof(World*));
		*w = world;
		luaL_getmetatable(_L, "World");
		lua_setmetatable(_L, -2);
	};

	World* get_world(int32_t index)
	{
		World* w = *(World**) CHECKUDATA(_L, index, "World");
		return w;
	};

	void push_physics_world(PhysicsWorld* world)
	{
		PhysicsWorld** w = (PhysicsWorld**) lua_newuserdata(_L, sizeof(PhysicsWorld*));
		luaL_getmetatable(_L, "PhysicsWorld");
		lua_setmetatable(_L, -2);
		*w = world;
	}

	PhysicsWorld* get_physics_world(int32_t index)
	{
		PhysicsWorld* w = *(PhysicsWorld**) CHECKUDATA(_L, index, "PhysicsWorld");
		return w;
	}

	void push_sound_world(SoundWorld* world)
	{
		SoundWorld** w = (SoundWorld**) lua_newuserdata(_L, sizeof(SoundWorld*));
		*w = world;
		luaL_getmetatable(_L, "SoundWorld");
		lua_setmetatable(_L, -2);
	}

	SoundWorld* get_sound_world(int32_t index)
	{
		SoundWorld* w = *(SoundWorld**) CHECKUDATA(_L, index, "SoundWorld");
		return w;
	}

	void push_unit(Unit* unit)
	{
		lua_pushlightuserdata(_L, unit);
	}

	Unit* get_unit(int32_t index)
	{
		return (Unit*) CHECKLIGHTDATA(_L, index, always_true, "Unit");
	}

	void push_camera(Camera* camera)
	{
		lua_pushlightuserdata(_L, camera);
	}

	Camera* get_camera(int32_t index)
	{
		return (Camera*) CHECKLIGHTDATA(_L, index, always_true, "Camera");
	}

	void push_mesh(Mesh* mesh)
	{
		lua_pushlightuserdata(_L, mesh);
	}

	Mesh* get_mesh(int32_t index)
	{
		return (Mesh*) CHECKLIGHTDATA(_L, index, always_true, "Mesh");
	}

	void push_sprite(Sprite* sprite)
	{
		lua_pushlightuserdata(_L, sprite);
	}

	Sprite* get_sprite(int32_t index)
	{
		return (Sprite*) CHECKLIGHTDATA(_L, index, always_true, "Sprite");
	}

	void push_material(Material* material)
	{
		lua_pushlightuserdata(_L, material);
	}

	Material* get_material(int32_t index)
	{
		return (Material*) CHECKLIGHTDATA(_L, index, always_true, "Material");
	}

	void push_actor(Actor* actor)
	{
		lua_pushlightuserdata(_L, actor);
	}

	Actor* get_actor(int32_t index)
	{
		return (Actor*) CHECKLIGHTDATA(_L, index, always_true, "Actor");
	}

	void push_controller(Controller* controller)
	{
		lua_pushlightuserdata(_L, controller);
	}

	Controller* get_controller(int32_t index)
	{
		return (Controller*) CHECKLIGHTDATA(_L, index, always_true, "Controller");
	}

	void push_raycast(Raycast* raycast)
	{
		lua_pushlightuserdata(_L, raycast);
	}

	Raycast* get_raycast(int32_t index)
	{
		return (Raycast*) CHECKLIGHTDATA(_L, index, always_true, "Raycast");
	}

	void push_sound_instance_id(const SoundInstanceId id)
	{
		uintptr_t enc = id.encode();
		lua_pushlightuserdata(_L, (void*)enc);
	}

	SoundInstanceId get_sound_instance_id(int32_t index)
	{
		uint32_t enc = (uintptr_t) CHECKLIGHTDATA(_L, index, always_true, "SoundInstanceId");
		SoundInstanceId id;
		id.decode(enc);
		return id;
	}

	void push_gui(Gui* gui)
	{
		lua_pushlightuserdata(_L, gui);
	}

	Gui* get_gui(int32_t index)
	{
		return (Gui*) CHECKLIGHTDATA(_L, index, always_true, "Gui");
	}

	void push_debug_line(DebugLine* line)
	{
		DebugLine** l = (DebugLine**) lua_newuserdata(_L, sizeof(DebugLine*));
		*l = line;
		luaL_getmetatable(_L, "DebugLine");
		lua_setmetatable(_L, -2);
	}

	DebugLine* get_debug_line(int32_t index)
	{
		DebugLine* l = *(DebugLine**) CHECKUDATA(_L, index, "DebugLine");
		return l;
	}

	Vector2& get_vector2(int32_t index)
	{
		void* v = CHECKLIGHTDATA(_L, index, lua_system::is_vector2, "Vector2");
		return *(Vector2*)v;
	}

	Vector3& get_vector3(int32_t index)
	{
		void* v = CHECKLIGHTDATA(_L, index, lua_system::is_vector3, "Vector3");
		return *(Vector3*)v;
	}

	Matrix4x4& get_matrix4x4(int32_t index)
	{
		void* m = CHECKLIGHTDATA(_L, index, lua_system::is_matrix4x4, "Matrix4x4");
		return *(Matrix4x4*)m;
	}

	Quaternion& get_quaternion(int32_t index)
	{
		void* q = CHECKLIGHTDATA(_L, index, lua_system::is_quaternion, "Quaternion");
		return *(Quaternion*)q;
	}

	Color4 get_color4(int32_t index)
	{
		// Color4 represented as Quaternion
		void* c = CHECKLIGHTDATA(_L, index, lua_system::is_quaternion, "Color4");
		Quaternion& q = *(Quaternion*)c;
		return Color4(q.x, q.y, q.z, q.w);
	}

	void push_vector2(const Vector2& v)
	{
		lua_pushlightuserdata(_L, lua_system::next_vector2(v));
		luaL_getmetatable(_L, "Lightuserdata_mt");
		lua_setmetatable(_L, -2);
	}

	void push_vector3(const Vector3& v)
	{
		lua_pushlightuserdata(_L, lua_system::next_vector3(v));
		luaL_getmetatable(_L, "Lightuserdata_mt");
		lua_setmetatable(_L, -2);
	}

	void push_matrix4x4(const Matrix4x4& m)
	{
		lua_pushlightuserdata(_L, lua_system::next_matrix4x4(m));
		luaL_getmetatable(_L, "Lightuserdata_mt");
		lua_setmetatable(_L, -2);
	}

	void push_quaternion(const Quaternion& q)
	{
		lua_pushlightuserdata(_L, lua_system::next_quaternion(q));
		luaL_getmetatable(_L, "Lightuserdata_mt");
		lua_setmetatable(_L, -2);
	}

	void push_vector2box(const Vector2& v)
	{
		Vector2* vec = (Vector2*) lua_newuserdata(_L, sizeof(Vector2));
		luaL_getmetatable(_L, "Vector2Box");
		lua_setmetatable(_L, -2);
		*vec = v;
	}

	Vector2& get_vector2box(uint32_t index)
	{
		Vector2* v = (Vector2*) CHECKUDATA(_L, index, "Vector2Box");
		return *v;
	}

	void push_vector3box(const Vector3& v)
	{
		Vector3* vec = (Vector3*) lua_newuserdata(_L, sizeof(Vector3));
		luaL_getmetatable(_L, "Vector3Box");
		lua_setmetatable(_L, -2);
		*vec = v;
	}

	Vector3& get_vector3box(uint32_t index)
	{
		Vector3* v = (Vector3*) CHECKUDATA(_L, index, "Vector3Box");
		return *v;
	}

	void push_quaternionbox(const Quaternion& q)
	{
		Quaternion* quat = (Quaternion*) lua_newuserdata(_L, sizeof(Quaternion));
		luaL_getmetatable(_L, "QuaternionBox");
		lua_setmetatable(_L, -2);
		*quat = q;
	}

	Quaternion& get_quaternionbox(uint32_t index)
	{
		Quaternion* q = (Quaternion*) CHECKUDATA(_L, index, "QuaternionBox");
		return *q;
	}

	void push_matrix4x4box(const Matrix4x4& m)
	{
		Matrix4x4* mat = (Matrix4x4*) lua_newuserdata(_L, sizeof(Matrix4x4));
		luaL_getmetatable(_L, "Matrix4x4Box");
		lua_setmetatable(_L, -2);
		*mat = m;
	}

	Matrix4x4& get_matrix4x4box(uint32_t index)
	{
		Matrix4x4* m = (Matrix4x4*) CHECKUDATA(_L, index, "Matrix4x4Box");
		return *m;
	}

private:

	lua_State* _L;
};

} // namespace crown
