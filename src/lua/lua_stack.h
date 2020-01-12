/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "core/math/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"
#include "world/types.h"
#include <lua.hpp>

#ifndef LUA_OK
	#define LUA_OK 0
#endif

#if CROWN_DEBUG
	#define LUA_ASSERT(condition, stack, msg, ...)                       \
		do                                                               \
		{                                                                \
			if (CE_UNLIKELY(!(condition)))                               \
			{                                                            \
				stack.push_fstring("Assertion failed: %s\n    " msg "\n" \
					, # condition                                        \
					, ## __VA_ARGS__                                     \
					);                                                   \
				lua_error(stack.L);                                      \
				CE_UNREACHABLE();                                        \
			}                                                            \
		}                                                                \
		while (0)
#else
	#define LUA_ASSERT(...) CE_NOOP()
#endif // CROWN_DEBUG

#define LIGHTDATA_TYPE_MASK      uintptr_t(0x3)
#define LIGHTDATA_TYPE_SHIFT     uintptr_t(0)

#define LIGHTDATA_POINTER_MARKER uintptr_t(0)

#define LIGHTDATA_UNIT_MARKER    uintptr_t(1)
#define LIGHTDATA_UNIT_ID_MASK   uintptr_t(0xfffffffc)
#define LIGHTDATA_UNIT_ID_SHIFT  uintptr_t(2)

#define LUA_VECTOR3_MARKER_MASK     uintptr_t(0x03)
#define LUA_VECTOR3_MARKER_SHIFT    uintptr_t(0)
#define LUA_QUATERNION_MARKER_MASK  uintptr_t(0x0f)
#define LUA_QUATERNION_MARKER_SHIFT uintptr_t(0)
#define LUA_MATRIX4X4_MARKER_MASK   uintptr_t(0x3f)
#define LUA_MATRIX4X4_MARKER_SHIFT  uintptr_t(0)

namespace crown
{
/// Wrapper to manipulate Lua stack.
///
/// @ingroup Lua
struct LuaStack
{
	lua_State* L;

	LuaStack(lua_State* L)
		: L(L)
	{
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

	bool is_bool(int i)
	{
		return lua_isboolean(L, i) == 1;
	}

	bool is_number(int i)
	{
		return lua_isnumber(L, i) == 1;
	}

	bool is_string(int i)
	{
		return lua_isstring(L, i) == 1;
	}

	bool is_pointer(int i)
	{
		return lua_islightuserdata(L, i) == 1
			&& ((uintptr_t)lua_touserdata(L, i) & LIGHTDATA_TYPE_MASK) == LIGHTDATA_POINTER_MARKER;
	}

	bool is_function(int i)
	{
		return lua_isfunction(L, i) == 1;
	}

	bool is_table(int i)
	{
		return lua_istable(L, i) == 1;
	}

	bool is_vector3(int i);
	bool is_quaternion(int i);
	bool is_matrix4x4(int i);

	/// Wraps lua_type.
	int value_type(int i)
	{
		return lua_type(L, i);
	}

	bool get_bool(int i)
	{
		return lua_toboolean(L, i) == 1;
	}

	int get_int(int i)
	{
#if CROWN_DEBUG
		return (int)luaL_checknumber(L, i);
#else
		return (int)lua_tonumber(L, i);
#endif
	}

	f32 get_float(int i)
	{
#if CROWN_DEBUG
		return (f32)luaL_checknumber(L, i);
#else
		return (f32)lua_tonumber(L, i);
#endif
	}

	const char* get_string(int i)
	{
#if CROWN_DEBUG
		return luaL_checkstring(L, i);
#else
		return lua_tostring(L, i);
#endif
	}

	void* get_pointer(int i)
	{
#if CROWN_DEBUG
		if (CE_UNLIKELY(lua_isuserdata(L, i) == 0))
		{
			luaL_typerror(L, i, "userdata");
			CE_UNREACHABLE();
		}
#endif
		void* p = lua_touserdata(L, i);
		CE_ENSURE(p != NULL); // NULL iff object is not userdata
		return p;
	}

	u32 get_id(int i)
	{
#if CROWN_DEBUG
		return (u32)luaL_checknumber(L, i);
#else
		return (u32)lua_tonumber(L, i);
#endif
	}

	StringId32 get_string_id_32(int i)
	{
		return StringId32(get_string(i));
	}

	StringId64 get_string_id_64(int i)
	{
		return StringId64(get_string(i));
	}

	StringId64 get_resource_name(int i)
	{
		return get_string_id_64(i);
	}

	Gui* get_gui(int i)
	{
		Gui* p = (Gui*)get_pointer(i);
		check_marker(i, p, DEBUG_GUI_MARKER, "Gui");
		return p;
	}

	DebugLine* get_debug_line(int i)
	{
		DebugLine* p = (DebugLine*)get_pointer(i);
		check_marker(i, p, DEBUG_LINE_MARKER, "DebugLine");
		return p;
	}

	ResourcePackage* get_resource_package(int i)
	{
		ResourcePackage* p = (ResourcePackage*)get_pointer(i);
		check_marker(i, p, RESOURCE_PACKAGE_MARKER, "ResourcePackage");
		return p;
	}

	World* get_world(int i)
	{
		World* p = (World*)get_pointer(i);
		check_marker(i, p, WORLD_MARKER, "World");
		return p;
	}

	SceneGraph* get_scene_graph(int i)
	{
		SceneGraph* p = (SceneGraph*)get_pointer(i);
		check_marker(i, p, SCENE_GRAPH_MARKER, "SceneGraph");
		return p;
	}

	Level* get_level(int i)
	{
		Level* p = (Level*)get_pointer(i);
		check_marker(i, p, LEVEL_MARKER, "Level");
		return p;
	}

	RenderWorld* get_render_world(int i)
	{
		RenderWorld* p = (RenderWorld*)get_pointer(i);
		check_marker(i, p, RENDER_WORLD_MARKER, "RenderWorld");
		return p;
	}

	PhysicsWorld* get_physics_world(int i)
	{
		PhysicsWorld* p = (PhysicsWorld*)get_pointer(i);
		check_marker(i, p, PHYSICS_WORLD_MARKER, "PhysicsWorld");
		return p;
	}

	SoundWorld* get_sound_world(int i)
	{
		SoundWorld* p = (SoundWorld*)get_pointer(i);
		check_marker(i, p, SOUND_WORLD_MARKER, "SoundWorld");
		return p;
	}

	ScriptWorld* get_script_world(int i)
	{
		ScriptWorld* p = (ScriptWorld*)get_pointer(i);
		check_marker(i, p, SCRIPT_WORLD_MARKER, "ScriptWorld");
		return p;
	}

	AnimationStateMachine* get_animation_state_machine(int i)
	{
		AnimationStateMachine* p = (AnimationStateMachine*)get_pointer(i);
		check_marker(i, p, ANIMATION_STATE_MACHINE_MARKER, "AnimationStateMachine");
		return p;
	}

	UnitId get_unit(int i)
	{
		uintptr_t enc = (uintptr_t)get_pointer(i);
#if CROWN_DEBUG
		if ((enc & LIGHTDATA_TYPE_MASK) != LIGHTDATA_UNIT_MARKER)
		{
			luaL_typerror(L, i, "UnitId");
			CE_UNREACHABLE();
		}
#endif // CROWN_DEBUG
		UnitId id;
		id._idx = u32((enc & LIGHTDATA_UNIT_ID_MASK) >> LIGHTDATA_UNIT_ID_SHIFT);
		return id;
	}

	CameraInstance get_camera(int i)
	{
		CameraInstance inst = { get_id(i) };
		return inst;
	}

	TransformInstance get_transform(int i)
	{
		TransformInstance inst = { get_id(i) };
		return inst;
	}

	MeshInstance get_mesh_instance(int i)
	{
		MeshInstance inst = { get_id(i) };
		return inst;
	}

	SpriteInstance get_sprite_instance(int i)
	{
		SpriteInstance inst = { get_id(i) };
		return inst;
	}

	LightInstance get_light_instance(int i)
	{
		LightInstance inst = { get_id(i) };
		return inst;
	}

	Material* get_material(int i)
	{
		return (Material*)get_pointer(i);
	}

	ActorInstance get_actor(int i)
	{
		ActorInstance inst = { get_id(i) };
		return inst;
	}

	SoundInstanceId get_sound_instance_id(int i)
	{
		return get_id(i);
	}

	ScriptInstance get_script_instance(int i)
	{
		ScriptInstance inst = { get_id(i) };
		return inst;
	}

	Vector2 get_vector2(int i)
	{
		Vector3 v = get_vector3(i);
		Vector2 a;
		a.x = v.x;
		a.y = v.y;
		return a;
	}

	Vector3& get_vector3(int i)
	{
#if CROWN_DEBUG
		return *check_temporary(i, (Vector3*)get_pointer(i));
#else
		return *(Vector3*)get_pointer(i);
#endif
	}

	Vector4 get_vector4(int i)
	{
		Quaternion q = get_quaternion(i);
		Vector4 a;
		a.x = q.x;
		a.y = q.y;
		a.z = q.z;
		a.w = q.w;
		return a;
	}

	Quaternion& get_quaternion(int i)
	{
#if CROWN_DEBUG
		return *check_temporary(i, (Quaternion*)get_pointer(i));
#else
		return *(Quaternion*)get_pointer(i);
#endif
	}

	Matrix4x4& get_matrix4x4(int i)
	{
#if CROWN_DEBUG
		return *check_temporary(i, (Matrix4x4*)get_pointer(i));
#else
		return *(Matrix4x4*)get_pointer(i);
#endif
	}

	Color4 get_color4(int i)
	{
		Quaternion q = get_quaternion(i);
		Color4 c;
		c.x = q.x;
		c.y = q.y;
		c.z = q.z;
		c.w = q.w;
		return c;
	}

	Vector2& get_vector2box(int i)
	{
		Vector2* v = (Vector2*)luaL_checkudata(L, i, "Vector2Box");
		return *v;
	}

	Vector3& get_vector3box(int i)
	{
		Vector3* v = (Vector3*)luaL_checkudata(L, i, "Vector3Box");
		return *v;
	}

	Quaternion& get_quaternionbox(int i)
	{
		Quaternion* q = (Quaternion*)luaL_checkudata(L, i, "QuaternionBox");
		return *q;
	}

	Matrix4x4& get_matrix4x4box(int i)
	{
		Matrix4x4* m = (Matrix4x4*)luaL_checkudata(L, i, "Matrix4x4Box");
		return *m;
	}

	void push_nil()
	{
		lua_pushnil(L);
	}

	void push_bool(bool value)
	{
		lua_pushboolean(L, value);
	}

	void push_int(int value)
	{
		lua_pushnumber(L, value);
	}

	void push_float(f32 value)
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

	void push_lstring(const char* s, u32 len)
	{
		lua_pushlstring(L, s, len);
	}

	void push_string_id(StringId32 value)
	{
		lua_pushnumber(L, value._id);
	}

	void push_pointer(void* p)
	{
		CE_ENSURE(NULL != p);
		lua_pushlightuserdata(L, p);
	}

	void push_function(lua_CFunction f)
	{
		lua_pushcfunction(L, f);
	}

	void push_id(u32 value)
	{
		lua_pushnumber(L, value);
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
	void push_table(int narr = 0, int nrec = 0)
	{
		lua_createtable(L, narr, nrec);
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

	void push_gui(Gui* dg)
	{
		push_pointer(dg);
	}

	void push_debug_line(DebugLine* line)
	{
		push_pointer(line);
	}

	void push_resource_package(ResourcePackage* package)
	{
		push_pointer(package);
	}

	void push_world(World* world)
	{
		push_pointer(world);
	};

	void push_scene_graph(SceneGraph* sg)
	{
		push_pointer(sg);
	}

	void push_level(Level* level)
	{
		push_pointer(level);
	}

	void push_render_world(RenderWorld* world)
	{
		push_pointer(world);
	}

	void push_physics_world(PhysicsWorld* world)
	{
		push_pointer(world);
	}

	void push_sound_world(SoundWorld* world)
	{
		push_pointer(world);
	}

	void push_script_world(ScriptWorld* world)
	{
		push_pointer(world);
	}

	void push_animation_state_machine(AnimationStateMachine* sm)
	{
		push_pointer(sm);
	}

	void push_unit(UnitId id)
	{
		uintptr_t enc = (uintptr_t(id._idx) << LIGHTDATA_UNIT_ID_SHIFT) | LIGHTDATA_UNIT_MARKER;
		push_pointer((void*)enc);
	}

	void push_camera(CameraInstance i)
	{
		push_id(i.i);
	}

	void push_transform(TransformInstance i)
	{
		push_id(i.i);
	}

	void push_mesh_instance(MeshInstance i)
	{
		push_id(i.i);
	}

	void push_sprite_instance(SpriteInstance i)
	{
		push_id(i.i);
	}

	void push_light_instance(LightInstance i)
	{
		push_id(i.i);
	}

	void push_material(Material* material)
	{
		push_pointer(material);
	}

	void push_actor(ActorInstance i)
	{
		push_id(i.i);
	}

	void push_sound_instance_id(SoundInstanceId id)
	{
		push_id(id);
	}

	void push_script_instance(ScriptInstance i)
	{
		push_id(i.i);
	}

	void push_vector2(const Vector2& v);
	void push_vector3(const Vector3& v);
	void push_matrix4x4(const Matrix4x4& m);
	void push_quaternion(const Quaternion& q);
	void push_color4(const Color4& c);

	void push_vector2box(const Vector2& v)
	{
		Vector2* vec = (Vector2*)lua_newuserdata(L, sizeof(Vector2));
		luaL_getmetatable(L, "Vector2Box");
		lua_setmetatable(L, -2);
		*vec = v;
	}

	void push_vector3box(const Vector3& v)
	{
		Vector3* vec = (Vector3*)lua_newuserdata(L, sizeof(Vector3));
		luaL_getmetatable(L, "Vector3Box");
		lua_setmetatable(L, -2);
		*vec = v;
	}

	void push_quaternionbox(const Quaternion& q)
	{
		Quaternion* quat = (Quaternion*)lua_newuserdata(L, sizeof(Quaternion));
		luaL_getmetatable(L, "QuaternionBox");
		lua_setmetatable(L, -2);
		*quat = q;
	}

	void push_matrix4x4box(const Matrix4x4& m)
	{
		Matrix4x4* mat = (Matrix4x4*)lua_newuserdata(L, sizeof(Matrix4x4));
		luaL_getmetatable(L, "Matrix4x4Box");
		lua_setmetatable(L, -2);
		*mat = m;
	}

	void push_value(int i)
	{
		lua_pushvalue(L, i);
	}

	void call(int nresults)
	{
		lua_pcall(L, 2, nresults, 0);
	}

#if CROWN_DEBUG
	Vector3* check_temporary(int i, const Vector3* p);
	Quaternion* check_temporary(int i, const Quaternion* p);
	Matrix4x4* check_temporary(int i, const Matrix4x4* p);

	void check_marker(int i, const void* p, u32 type_marker, const char* type_name)
	{
		if (CE_UNLIKELY(!is_pointer(i) || *(u32*)p != type_marker))
		{
			luaL_typerror(L, i, type_name);
			CE_UNREACHABLE();
		}
	}
#else
	void check_marker(int /*i*/, const void* /*p*/, u32 /*type_marker*/, const char* /*type_name*/)
	{
	}
#endif // CROWN_DEBUG
};

} // namespace crown
