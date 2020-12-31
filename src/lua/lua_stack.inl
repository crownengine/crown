/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/error/error.inl"
#include "lua/lua_stack.h"

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

namespace crown
{
inline LuaStack::LuaStack(lua_State* L)
	: L(L)
{
}

inline int LuaStack::num_args()
{
	return lua_gettop(L);
}

inline void LuaStack::remove(int i)
{
	lua_remove(L, i);
}

inline void LuaStack::pop(int n)
{
	lua_pop(L, n);
}

inline bool LuaStack::is_nil(int i)
{
	return lua_isnil(L, i) == 1;
}

inline bool LuaStack::is_bool(int i)
{
	return lua_isboolean(L, i) == 1;
}

inline bool LuaStack::is_number(int i)
{
	return lua_isnumber(L, i) == 1;
}

inline bool LuaStack::is_string(int i)
{
	return lua_isstring(L, i) == 1;
}

inline bool LuaStack::is_pointer(int i)
{
	return lua_islightuserdata(L, i) == 1
		&& ((uintptr_t)lua_touserdata(L, i) & LIGHTDATA_TYPE_MASK) == LIGHTDATA_POINTER_MARKER;
}

inline bool LuaStack::is_function(int i)
{
	return lua_isfunction(L, i) == 1;
}

inline bool LuaStack::is_table(int i)
{
	return lua_istable(L, i) == 1;
}

inline int LuaStack::value_type(int i)
{
	return lua_type(L, i);
}

inline bool LuaStack::get_bool(int i)
{
	return lua_toboolean(L, i) == 1;
}

inline int LuaStack::get_int(int i)
{
#if CROWN_DEBUG
	return (int)luaL_checknumber(L, i);
#else
	return (int)lua_tonumber(L, i);
#endif
}

inline f32 LuaStack::get_float(int i)
{
#if CROWN_DEBUG
	return (f32)luaL_checknumber(L, i);
#else
	return (f32)lua_tonumber(L, i);
#endif
}

inline const char* LuaStack::get_string(int i)
{
#if CROWN_DEBUG
	return luaL_checkstring(L, i);
#else
	return lua_tostring(L, i);
#endif
}

inline void* LuaStack::get_pointer(int i)
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

inline u32 LuaStack::get_id(int i)
{
#if CROWN_DEBUG
	return (u32)luaL_checknumber(L, i);
#else
	return (u32)lua_tonumber(L, i);
#endif
}

inline StringId32 LuaStack::get_string_id_32(int i)
{
	return StringId32(get_string(i));
}

inline StringId64 LuaStack::get_string_id_64(int i)
{
	return StringId64(get_string(i));
}

inline StringId64 LuaStack::get_resource_name(int i)
{
	return get_string_id_64(i);
}

inline Gui* LuaStack::get_gui(int i)
{
	Gui* p = (Gui*)get_pointer(i);
	check_marker(i, p, DEBUG_GUI_MARKER, "Gui");
	return p;
}

inline DebugLine* LuaStack::get_debug_line(int i)
{
	DebugLine* p = (DebugLine*)get_pointer(i);
	check_marker(i, p, DEBUG_LINE_MARKER, "DebugLine");
	return p;
}

inline ResourcePackage* LuaStack::get_resource_package(int i)
{
	ResourcePackage* p = (ResourcePackage*)get_pointer(i);
	check_marker(i, p, RESOURCE_PACKAGE_MARKER, "ResourcePackage");
	return p;
}

inline World* LuaStack::get_world(int i)
{
	World* p = (World*)get_pointer(i);
	check_marker(i, p, WORLD_MARKER, "World");
	return p;
}

inline SceneGraph* LuaStack::get_scene_graph(int i)
{
	SceneGraph* p = (SceneGraph*)get_pointer(i);
	check_marker(i, p, SCENE_GRAPH_MARKER, "SceneGraph");
	return p;
}

inline Level* LuaStack::get_level(int i)
{
	Level* p = (Level*)get_pointer(i);
	check_marker(i, p, LEVEL_MARKER, "Level");
	return p;
}

inline RenderWorld* LuaStack::get_render_world(int i)
{
	RenderWorld* p = (RenderWorld*)get_pointer(i);
	check_marker(i, p, RENDER_WORLD_MARKER, "RenderWorld");
	return p;
}

inline PhysicsWorld* LuaStack::get_physics_world(int i)
{
	PhysicsWorld* p = (PhysicsWorld*)get_pointer(i);
	check_marker(i, p, PHYSICS_WORLD_MARKER, "PhysicsWorld");
	return p;
}

inline SoundWorld* LuaStack::get_sound_world(int i)
{
	SoundWorld* p = (SoundWorld*)get_pointer(i);
	check_marker(i, p, SOUND_WORLD_MARKER, "SoundWorld");
	return p;
}

inline ScriptWorld* LuaStack::get_script_world(int i)
{
	ScriptWorld* p = (ScriptWorld*)get_pointer(i);
	check_marker(i, p, SCRIPT_WORLD_MARKER, "ScriptWorld");
	return p;
}

inline AnimationStateMachine* LuaStack::get_animation_state_machine(int i)
{
	AnimationStateMachine* p = (AnimationStateMachine*)get_pointer(i);
	check_marker(i, p, ANIMATION_STATE_MACHINE_MARKER, "AnimationStateMachine");
	return p;
}

inline UnitId LuaStack::get_unit(int i)
{
	uintptr_t enc = (uintptr_t)get_pointer(i);
#if CROWN_DEBUG
	if ((enc & LIGHTDATA_TYPE_MASK) != LIGHTDATA_UNIT_MARKER)
	{
		luaL_typerror(L, i, "UnitId");
		CE_UNREACHABLE();
	}
#endif // CROWN_DEBUG
	UnitId unit;
	unit._idx = u32((enc & LIGHTDATA_UNIT_ID_MASK) >> LIGHTDATA_UNIT_ID_SHIFT);
	return unit;
}

inline CameraInstance LuaStack::get_camera_instance(int i)
{
	CameraInstance inst = { get_id(i) };
	return inst;
}

inline TransformInstance LuaStack::get_transform_instance(int i)
{
	TransformInstance inst = { get_id(i) };
	return inst;
}

inline MeshInstance LuaStack::get_mesh_instance(int i)
{
	MeshInstance inst = { get_id(i) };
	return inst;
}

inline SpriteInstance LuaStack::get_sprite_instance(int i)
{
	SpriteInstance inst = { get_id(i) };
	return inst;
}

inline LightInstance LuaStack::get_light_instance(int i)
{
	LightInstance inst = { get_id(i) };
	return inst;
}

inline StateMachineInstance LuaStack::get_state_machine_instance(int i)
{
	StateMachineInstance inst = { get_id(i) };
	return inst;
}

inline Material* LuaStack::get_material(int i)
{
	return (Material*)get_pointer(i);
}

inline ActorInstance LuaStack::get_actor_instance(int i)
{
	ActorInstance inst = { get_id(i) };
	return inst;
}

inline SoundInstanceId LuaStack::get_sound_instance_id(int i)
{
	return get_id(i);
}

inline ScriptInstance LuaStack::get_script_instance(int i)
{
	ScriptInstance inst = { get_id(i) };
	return inst;
}

inline Vector2 LuaStack::get_vector2(int i)
{
	Vector3 v = get_vector3(i);
	Vector2 a;
	a.x = v.x;
	a.y = v.y;
	return a;
}

inline Vector3& LuaStack::get_vector3(int i)
{
#if CROWN_DEBUG
	return *check_temporary(i, (Vector3*)get_pointer(i));
#else
	return *(Vector3*)get_pointer(i);
#endif
}

inline Vector4 LuaStack::get_vector4(int i)
{
	Quaternion q = get_quaternion(i);
	Vector4 a;
	a.x = q.x;
	a.y = q.y;
	a.z = q.z;
	a.w = q.w;
	return a;
}

inline Quaternion& LuaStack::get_quaternion(int i)
{
#if CROWN_DEBUG
	return *check_temporary(i, (Quaternion*)get_pointer(i));
#else
	return *(Quaternion*)get_pointer(i);
#endif
}

inline Matrix4x4& LuaStack::get_matrix4x4(int i)
{
#if CROWN_DEBUG
	return *check_temporary(i, (Matrix4x4*)get_pointer(i));
#else
	return *(Matrix4x4*)get_pointer(i);
#endif
}

inline Color4 LuaStack::get_color4(int i)
{
	Quaternion q = get_quaternion(i);
	Color4 c;
	c.x = q.x;
	c.y = q.y;
	c.z = q.z;
	c.w = q.w;
	return c;
}

inline Vector2& LuaStack::get_vector2box(int i)
{
	Vector2* v = (Vector2*)luaL_checkudata(L, i, "Vector2Box");
	return *v;
}

inline Vector3& LuaStack::get_vector3box(int i)
{
	Vector3* v = (Vector3*)luaL_checkudata(L, i, "Vector3Box");
	return *v;
}

inline Quaternion& LuaStack::get_quaternionbox(int i)
{
	Quaternion* q = (Quaternion*)luaL_checkudata(L, i, "QuaternionBox");
	return *q;
}

inline Matrix4x4& LuaStack::get_matrix4x4box(int i)
{
	Matrix4x4* m = (Matrix4x4*)luaL_checkudata(L, i, "Matrix4x4Box");
	return *m;
}

inline void LuaStack::push_nil()
{
	lua_pushnil(L);
}

inline void LuaStack::push_bool(bool value)
{
	lua_pushboolean(L, value);
}

inline void LuaStack::push_int(int value)
{
	lua_pushnumber(L, value);
}

inline void LuaStack::push_float(f32 value)
{
	lua_pushnumber(L, value);
}

inline void LuaStack::push_string(const char* s)
{
	lua_pushstring(L, s);
}

inline void LuaStack::push_fstring(const char* fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	lua_pushvfstring(L, fmt, vl);
	va_end(vl);
}

inline void LuaStack::push_lstring(const char* s, u32 len)
{
	lua_pushlstring(L, s, len);
}

inline void LuaStack::push_string_id(StringId32 value)
{
	lua_pushnumber(L, value._id);
}

inline void LuaStack::push_pointer(void* p)
{
	CE_ENSURE(NULL != p);
	lua_pushlightuserdata(L, p);
}

inline void LuaStack::push_function(lua_CFunction f)
{
	lua_pushcfunction(L, f);
}

inline void LuaStack::push_id(u32 value)
{
	lua_pushnumber(L, value);
}

inline void LuaStack::push_table(int narr, int nrec)
{
	lua_createtable(L, narr, nrec);
}

inline void LuaStack::push_key_begin(const char* key)
{
	lua_pushstring(L, key);
}

inline void LuaStack::push_key_begin(int i)
{
	lua_pushnumber(L, i);
}

inline void LuaStack::push_key_end()
{
	lua_settable(L, -3);
}

inline int LuaStack::next(int i)
{
	return lua_next(L, i);
}

inline void LuaStack::push_gui(Gui* dg)
{
	push_pointer(dg);
}

inline void LuaStack::push_debug_line(DebugLine* line)
{
	push_pointer(line);
}

inline void LuaStack::push_resource_package(ResourcePackage* package)
{
	push_pointer(package);
}

inline void LuaStack::push_world(World* world)
{
	push_pointer(world);
}

inline void LuaStack::push_scene_graph(SceneGraph* sg)
{
	push_pointer(sg);
}

inline void LuaStack::push_level(Level* level)
{
	push_pointer(level);
}

inline void LuaStack::push_render_world(RenderWorld* world)
{
	push_pointer(world);
}

inline void LuaStack::push_physics_world(PhysicsWorld* world)
{
	push_pointer(world);
}

inline void LuaStack::push_sound_world(SoundWorld* world)
{
	push_pointer(world);
}

inline void LuaStack::push_script_world(ScriptWorld* world)
{
	push_pointer(world);
}

inline void LuaStack::push_animation_state_machine(AnimationStateMachine* sm)
{
	push_pointer(sm);
}

inline void LuaStack::push_unit(UnitId unit)
{
	uintptr_t enc = (uintptr_t(unit._idx) << LIGHTDATA_UNIT_ID_SHIFT) | LIGHTDATA_UNIT_MARKER;
	push_pointer((void*)enc);
}

inline void LuaStack::push_camera(CameraInstance i)
{
	push_id(i.i);
}

inline void LuaStack::push_transform(TransformInstance i)
{
	push_id(i.i);
}

inline void LuaStack::push_mesh_instance(MeshInstance i)
{
	push_id(i.i);
}

inline void LuaStack::push_sprite_instance(SpriteInstance i)
{
	push_id(i.i);
}

inline void LuaStack::push_light_instance(LightInstance i)
{
	push_id(i.i);
}

inline void LuaStack::push_state_machine_instance(StateMachineInstance i)
{
	push_id(i.i);
}

inline void LuaStack::push_material(Material* material)
{
	push_pointer(material);
}

inline void LuaStack::push_actor(ActorInstance i)
{
	push_id(i.i);
}

inline void LuaStack::push_sound_instance_id(SoundInstanceId id)
{
	push_id(id);
}

inline void LuaStack::push_script_instance(ScriptInstance i)
{
	push_id(i.i);
}

inline void LuaStack::push_vector2(const Vector2& v)
{
	Vector3 a;
	a.x = v.x;
	a.y = v.y;
	a.z = 0.0f;
	push_vector3(a);
}

inline void LuaStack::push_color4(const Color4& c)
{
	Quaternion q;
	q.x = c.x;
	q.y = c.y;
	q.z = c.z;
	q.w = c.w;
	push_quaternion(q);
}

inline void LuaStack::push_vector2box(const Vector2& v)
{
	Vector2* vec = (Vector2*)lua_newuserdata(L, sizeof(Vector2));
	luaL_getmetatable(L, "Vector2Box");
	lua_setmetatable(L, -2);
	*vec = v;
}

inline void LuaStack::push_vector3box(const Vector3& v)
{
	Vector3* vec = (Vector3*)lua_newuserdata(L, sizeof(Vector3));
	luaL_getmetatable(L, "Vector3Box");
	lua_setmetatable(L, -2);
	*vec = v;
}

inline void LuaStack::push_quaternionbox(const Quaternion& q)
{
	Quaternion* quat = (Quaternion*)lua_newuserdata(L, sizeof(Quaternion));
	luaL_getmetatable(L, "QuaternionBox");
	lua_setmetatable(L, -2);
	*quat = q;
}

inline void LuaStack::push_matrix4x4box(const Matrix4x4& m)
{
	Matrix4x4* mat = (Matrix4x4*)lua_newuserdata(L, sizeof(Matrix4x4));
	luaL_getmetatable(L, "Matrix4x4Box");
	lua_setmetatable(L, -2);
	*mat = m;
}

inline void LuaStack::push_value(int i)
{
	lua_pushvalue(L, i);
}

inline void LuaStack::call(int nresults)
{
	lua_pcall(L, 2, nresults, 0);
}

#if CROWN_DEBUG
inline void LuaStack::check_marker(int i, const void* p, u32 type_marker, const char* type_name)
{
	if (CE_UNLIKELY(!is_pointer(i) || *(u32*)p != type_marker))
	{
		luaL_typerror(L, i, type_name);
		CE_UNREACHABLE();
	}
}
#else
inline void LuaStack::check_marker(int /*i*/, const void* /*p*/, u32 /*type_marker*/, const char* /*type_name*/)
{
}
#endif // CROWN_DEBUG

} // namespace crown
