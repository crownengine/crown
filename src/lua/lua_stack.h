/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "vector3.h"
#include "vector2.h"
#include "quaternion.h"
#include "matrix4x4.h"
#include "string_utils.h"
#include "color4.h"
#include <lua.hpp>

#if CROWN_DEBUG
	static void* checkudata(lua_State* L, int i, const char* expected)
	{
		luaL_checktype(L, i, LUA_TUSERDATA);
		return luaL_checkudata(L, i, expected);
	}

	typedef bool (*checkfn)(int);
	static void* checklightdata(lua_State* L, int i, checkfn cf, const char* expected)
	{
		luaL_checktype(L, i, LUA_TLIGHTUSERDATA);
		if (!cf(i)) luaL_typerror(L, i, expected);
		return lua_touserdata(L, i);
	}

	static bool always_true(int i)
	{
		return i == i;
	}

	#define CHECKUDATA(stack, i, expected) checkudata(stack, i, expected)
	#define CHECKLIGHTDATA(stack, i, cf, expected) checklightdata(stack, i, cf, expected)
	#define CHECKBOOLEAN(stack, i) lua_toboolean(stack, i)
	#define CHECKINTEGER(stack, i) luaL_checkinteger(stack, i)
	#define CHECKNUMBER(stack, i) luaL_checknumber(stack, i)
	#define CHECKSTRING(stack, i) luaL_checkstring(stack, i)

	#define LUA_ASSERT(condition, stack, msg, ...) do { if (!(condition)) {\
		stack.push_fstring("\nLua assertion failed: %s\n\t" msg "\n", #condition, ##__VA_ARGS__); lua_error(stack.state()); }} while (0);
#else
	#define CHECKUDATA(stack, i, expected) lua_touserdata(stack, i)
	#define CHECKLIGHTDATA(stack, i, cf, expected) lua_touserdata(stack, i)
	#define CHECKBOOLEAN(stack, i) lua_toboolean(stack, i)
	#define CHECKINTEGER(stack, i) lua_tointeger(stack, i)
	#define CHECKNUMBER(stack, i) lua_tonumber(stack, i)
	#define CHECKSTRING(stack, i) lua_tostring(stack, i)

	#define LUA_ASSERT(...) ((void)0)
#endif // CROWN_DEBUG

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
		: L(L)
	{
	}

	lua_State* state()
	{
		return L;
	}

	/// Returns the number of elements in the stack.
	/// When called inside a function, it can be used to count
	/// the number of arguments passed to the function itself.
	int num_args()
	{
		return lua_gettop(L);
	}

	/// Removes the element at the given valid index, shifting down the elements
	/// above this index to fill the gap. Cannot be called with a pseudo-index,
	/// because a pseudo-index is not an actual stack position.
	void remove(int i)
	{
		lua_remove(L, i);
	}

	/// Pops @a n elements from the stack.
	void pop(int n)
	{
		lua_pop(L, n);
	}
	bool is_nil(int i)
	{
		return lua_isnil(L, i) == 1;
	}

	bool is_number(int i)
	{
		return lua_isnumber(L, i) == 1;
	}

	/// Wraps lua_type.
	int value_type(int i)
	{
		return lua_type(L, i);
	}

	void push_nil()
	{
		lua_pushnil(L);
	}

	void push_bool(bool value)
	{
		lua_pushboolean(L, value);
	}

	void push_int32(int32_t value)
	{
		lua_pushinteger(L, value);
	}

	void push_uint32(uint32_t value)
	{
		lua_pushinteger(L, value);
	}

	void push_string_id(StringId32 value)
	{
		lua_pushinteger(L, value.id());
	}

	void push_float(float value)
	{
		lua_pushnumber(L, value);
	}

	void push_string(const char* s)
	{
		lua_pushstring(L, s);
	}

	void push_fstring(const char* fmt, ...)
	{
		va_list vl;
		va_start(vl, fmt);
		lua_pushvfstring(L, fmt, vl);
		va_end(vl);
	}

	void push_literal(const char* s, uint32_t len)
	{
		lua_pushlstring(L, s, len);
	}

	bool get_bool(int i)
	{
		return CHECKBOOLEAN(L, i) == 1;
	}

	int get_int(int i)
	{
		return (int)CHECKINTEGER(L, i);
	}

	StringId32 get_string_id(int i)
	{
		return StringId32(uint32_t(CHECKINTEGER(L, i)));
	}

	float get_float(int i)
	{
		return (float) CHECKNUMBER(L, i);
	}

	const char* get_string(int i)
	{
		return CHECKSTRING(L, i);
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
		lua_newtable(L);
	}

	/// See Stack::push_table()
	void push_key_begin(const char* key)
	{
		lua_pushstring(L, key);
	}

	/// See Stack::push_table()
	void push_key_begin(int i)
	{
		lua_pushnumber(L, i);
	}

	/// See Stack::push_table()
	void push_key_end()
	{
		lua_settable(L, -3);
	}

	int next(int i)
	{
		return lua_next(L, i);
	}

	StringId64 get_resource_id(int i)
	{
		return StringId64(CHECKSTRING(L, i));
	}

	void push_resource_package(ResourcePackage* package)
	{
		ResourcePackage** p = (ResourcePackage**) lua_newuserdata(L, sizeof(ResourcePackage*));
		*p = package;
		luaL_getmetatable(L, "ResourcePackage");
		lua_setmetatable(L, -2);
	}

	ResourcePackage* get_resource_package(int i)
	{
		ResourcePackage* pkg = *(ResourcePackage**) CHECKUDATA(L, i, "ResourcePackage");
		return pkg;
	}

	void push_world(World* world)
	{
		World** w = (World**) lua_newuserdata(L, sizeof(World*));
		*w = world;
		luaL_getmetatable(L, "World");
		lua_setmetatable(L, -2);
	};

	World* get_world(int i)
	{
		World* w = *(World**) CHECKUDATA(L, i, "World");
		return w;
	};

	void push_physics_world(PhysicsWorld* world)
	{
		PhysicsWorld** w = (PhysicsWorld**) lua_newuserdata(L, sizeof(PhysicsWorld*));
		luaL_getmetatable(L, "PhysicsWorld");
		lua_setmetatable(L, -2);
		*w = world;
	}

	PhysicsWorld* get_physics_world(int i)
	{
		PhysicsWorld* w = *(PhysicsWorld**) CHECKUDATA(L, i, "PhysicsWorld");
		return w;
	}

	void push_sound_world(SoundWorld* world)
	{
		SoundWorld** w = (SoundWorld**) lua_newuserdata(L, sizeof(SoundWorld*));
		*w = world;
		luaL_getmetatable(L, "SoundWorld");
		lua_setmetatable(L, -2);
	}

	SoundWorld* get_sound_world(int i)
	{
		SoundWorld* w = *(SoundWorld**) CHECKUDATA(L, i, "SoundWorld");
		return w;
	}

	void push_unit(Unit* unit)
	{
		lua_pushlightuserdata(L, unit);
	}

	Unit* get_unit(int i)
	{
		return (Unit*) CHECKLIGHTDATA(L, i, always_true, "Unit");
	}

	void push_camera(Camera* camera)
	{
		lua_pushlightuserdata(L, camera);
	}

	Camera* get_camera(int i)
	{
		return (Camera*) CHECKLIGHTDATA(L, i, always_true, "Camera");
	}

	void push_mesh(Mesh* mesh)
	{
		lua_pushlightuserdata(L, mesh);
	}

	Mesh* get_mesh(int i)
	{
		return (Mesh*) CHECKLIGHTDATA(L, i, always_true, "Mesh");
	}

	void push_sprite(Sprite* sprite)
	{
		lua_pushlightuserdata(L, sprite);
	}

	Sprite* get_sprite(int i)
	{
		return (Sprite*) CHECKLIGHTDATA(L, i, always_true, "Sprite");
	}

	void push_material(Material* material)
	{
		lua_pushlightuserdata(L, material);
	}

	Material* get_material(int i)
	{
		return (Material*) CHECKLIGHTDATA(L, i, always_true, "Material");
	}

	void push_actor(Actor* actor)
	{
		lua_pushlightuserdata(L, actor);
	}

	Actor* get_actor(int i)
	{
		return (Actor*) CHECKLIGHTDATA(L, i, always_true, "Actor");
	}

	void push_controller(Controller* controller)
	{
		lua_pushlightuserdata(L, controller);
	}

	Controller* get_controller(int i)
	{
		return (Controller*) CHECKLIGHTDATA(L, i, always_true, "Controller");
	}

	void push_raycast(Raycast* raycast)
	{
		lua_pushlightuserdata(L, raycast);
	}

	Raycast* get_raycast(int i)
	{
		return (Raycast*) CHECKLIGHTDATA(L, i, always_true, "Raycast");
	}

	void push_sound_instance_id(const SoundInstanceId id)
	{
		uintptr_t enc = id.encode();
		lua_pushlightuserdata(L, (void*)enc);
	}

	SoundInstanceId get_sound_instance_id(int i)
	{
		uint32_t enc = (uintptr_t) CHECKLIGHTDATA(L, i, always_true, "SoundInstanceId");
		SoundInstanceId id;
		id.decode(enc);
		return id;
	}

	void push_gui(Gui* gui)
	{
		lua_pushlightuserdata(L, gui);
	}

	Gui* get_gui(int i)
	{
		return (Gui*) CHECKLIGHTDATA(L, i, always_true, "Gui");
	}

	void push_debug_line(DebugLine* line)
	{
		DebugLine** l = (DebugLine**) lua_newuserdata(L, sizeof(DebugLine*));
		*l = line;
		luaL_getmetatable(L, "DebugLine");
		lua_setmetatable(L, -2);
	}

	DebugLine* get_debug_line(int i)
	{
		DebugLine* l = *(DebugLine**) CHECKUDATA(L, i, "DebugLine");
		return l;
	}

	Vector2 get_vector2(int i);
	Vector3& get_vector3(int i);
	Matrix4x4& get_matrix4x4(int i);
	Quaternion& get_quaternion(int i);
	Color4 get_color4(int i);
	void push_vector2(const Vector2& v);
	void push_vector3(const Vector3& v);
	void push_matrix4x4(const Matrix4x4& m);
	void push_quaternion(const Quaternion& q);

	void push_vector2box(const Vector2& v)
	{
		Vector2* vec = (Vector2*) lua_newuserdata(L, sizeof(Vector2));
		luaL_getmetatable(L, "Vector2Box");
		lua_setmetatable(L, -2);
		*vec = v;
	}

	Vector2& get_vector2box(int i)
	{
		Vector2* v = (Vector2*) CHECKUDATA(L, i, "Vector2Box");
		return *v;
	}

	void push_vector3box(const Vector3& v)
	{
		Vector3* vec = (Vector3*) lua_newuserdata(L, sizeof(Vector3));
		luaL_getmetatable(L, "Vector3Box");
		lua_setmetatable(L, -2);
		*vec = v;
	}

	Vector3& get_vector3box(int i)
	{
		Vector3* v = (Vector3*) CHECKUDATA(L, i, "Vector3Box");
		return *v;
	}

	void push_quaternionbox(const Quaternion& q)
	{
		Quaternion* quat = (Quaternion*) lua_newuserdata(L, sizeof(Quaternion));
		luaL_getmetatable(L, "QuaternionBox");
		lua_setmetatable(L, -2);
		*quat = q;
	}

	Quaternion& get_quaternionbox(int i)
	{
		Quaternion* q = (Quaternion*) CHECKUDATA(L, i, "QuaternionBox");
		return *q;
	}

	void push_matrix4x4box(const Matrix4x4& m)
	{
		Matrix4x4* mat = (Matrix4x4*) lua_newuserdata(L, sizeof(Matrix4x4));
		luaL_getmetatable(L, "Matrix4x4Box");
		lua_setmetatable(L, -2);
		*mat = m;
	}

	Matrix4x4& get_matrix4x4box(int i)
	{
		Matrix4x4* m = (Matrix4x4*) CHECKUDATA(L, i, "Matrix4x4Box");
		return *m;
	}

private:

	lua_State* L;
};

} // namespace crown
