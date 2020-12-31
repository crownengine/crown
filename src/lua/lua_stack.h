/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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

	///
	LuaStack(lua_State* L);

	/// Returns the number of elements in the stack.
	/// When called inside a function, it can be used to count
	/// the number of arguments passed to the function itself.
	int num_args();

	/// Removes the element at the given valid index, shifting down the elements
	/// above this index to fill the gap. Cannot be called with a pseudo-index,
	/// because a pseudo-index is not an actual stack position.
	void remove(int i);

	/// Pops @a n elements from the stack.
	void pop(int n);

	///
	bool is_nil(int i);

	///
	bool is_bool(int i);

	///
	bool is_number(int i);

	///
	bool is_string(int i);

	///
	bool is_pointer(int i);

	///
	bool is_function(int i);

	///
	bool is_table(int i);

	///
	bool is_vector3(int i);

	///
	bool is_quaternion(int i);

	///
	bool is_matrix4x4(int i);

	/// Wraps lua_type.
	int value_type(int i);

	///
	bool get_bool(int i);

	///
	int get_int(int i);

	///
	f32 get_float(int i);

	///
	const char* get_string(int i);

	///
	void* get_pointer(int i);

	///
	u32 get_id(int i);

	///
	StringId32 get_string_id_32(int i);

	///
	StringId64 get_string_id_64(int i);

	///
	StringId64 get_resource_name(int i);

	///
	Gui* get_gui(int i);

	///
	DebugLine* get_debug_line(int i);

	///
	ResourcePackage* get_resource_package(int i);

	///
	World* get_world(int i);

	///
	SceneGraph* get_scene_graph(int i);

	///
	Level* get_level(int i);

	///
	RenderWorld* get_render_world(int i);

	///
	PhysicsWorld* get_physics_world(int i);

	///
	SoundWorld* get_sound_world(int i);

	///
	ScriptWorld* get_script_world(int i);

	///
	AnimationStateMachine* get_animation_state_machine(int i);

	///
	UnitId get_unit(int i);

	///
	CameraInstance get_camera_instance(int i);

	///
	TransformInstance get_transform_instance(int i);

	///
	MeshInstance get_mesh_instance(int i);

	///
	SpriteInstance get_sprite_instance(int i);

	///
	LightInstance get_light_instance(int i);

	///
	StateMachineInstance get_state_machine_instance(int i);

	///
	Material* get_material(int i);

	///
	ActorInstance get_actor_instance(int i);

	///
	SoundInstanceId get_sound_instance_id(int i);

	///
	ScriptInstance get_script_instance(int i);

	///
	Vector2 get_vector2(int i);

	///
	Vector3& get_vector3(int i);

	///
	Vector4 get_vector4(int i);

	///
	Quaternion& get_quaternion(int i);

	///
	Matrix4x4& get_matrix4x4(int i);

	///
	Color4 get_color4(int i);

	///
	Vector2& get_vector2box(int i);

	///
	Vector3& get_vector3box(int i);

	///
	Quaternion& get_quaternionbox(int i);

	///
	Matrix4x4& get_matrix4x4box(int i);

	///
	void push_nil();

	///
	void push_bool(bool value);

	///
	void push_int(int value);

	///
	void push_float(f32 value);

	///
	void push_string(const char* s);

	///
	void push_fstring(const char* fmt, ...);

	///
	void push_lstring(const char* s, u32 len);

	///
	void push_string_id(StringId32 value);

	///
	void push_pointer(void* p);

	///
	void push_function(lua_CFunction f);

	///
	void push_id(u32 value);

	/// Pushes an empty table onto the stack.
	/// When you want to set keys on the table, you have to use LuaStack::push_key_begin()
	/// and LuaStack::push_key_end() as in the following example:
	///
	/// LuaStack stack(L)
	/// stack.push_table()
	/// stack.push_key_begin("foo"); stack.push_foo(); stack.push_key_end()
	/// stack.push_key_begin("bar"); stack.push_bar(); stack.push_key_end()
	/// return 1;
	void push_table(int narr = 0, int nrec = 0);

	/// See Stack::push_table()
	void push_key_begin(const char* key);

	/// See Stack::push_table()
	void push_key_begin(int i);

	/// See Stack::push_table()
	void push_key_end();

	///
	int next(int i);

	///
	void push_gui(Gui* dg);

	///
	void push_debug_line(DebugLine* line);

	///
	void push_resource_package(ResourcePackage* package);

	///
	void push_world(World* world);

	///
	void push_scene_graph(SceneGraph* sg);

	///
	void push_level(Level* level);

	///
	void push_render_world(RenderWorld* world);

	///
	void push_physics_world(PhysicsWorld* world);

	///
	void push_sound_world(SoundWorld* world);

	///
	void push_script_world(ScriptWorld* world);

	///
	void push_animation_state_machine(AnimationStateMachine* sm);

	///
	void push_unit(UnitId unit);

	///
	void push_camera(CameraInstance i);

	///
	void push_transform(TransformInstance i);

	///
	void push_mesh_instance(MeshInstance i);

	///
	void push_sprite_instance(SpriteInstance i);

	///
	void push_light_instance(LightInstance i);

	///
	void push_state_machine_instance(StateMachineInstance i);

	///
	void push_material(Material* material);

	///
	void push_actor(ActorInstance i);

	///
	void push_sound_instance_id(SoundInstanceId id);

	///
	void push_script_instance(ScriptInstance i);

	///
	void push_vector2(const Vector2& v);

	///
	void push_vector3(const Vector3& v);

	///
	void push_matrix4x4(const Matrix4x4& m);

	///
	void push_quaternion(const Quaternion& q);

	///
	void push_color4(const Color4& c);

	///
	void push_vector2box(const Vector2& v);

	///
	void push_vector3box(const Vector3& v);

	///
	void push_quaternionbox(const Quaternion& q);

	///
	void push_matrix4x4box(const Matrix4x4& m);

	///
	void push_value(int i);

	///
	void call(int nresults);

	///
	Vector3* check_temporary(int i, const Vector3* p);

	///
	Quaternion* check_temporary(int i, const Quaternion* p);

	///
	Matrix4x4* check_temporary(int i, const Matrix4x4* p);

	///
	void check_marker(int i, const void* p, u32 type_marker, const char* type_name);
};

} // namespace crown
