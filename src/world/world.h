/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/constants.h"
#include "core/math/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "lua/types.h"
#include "resource/types.h"
#include "world/event_stream.h"
#include "world/gui.h"
#include "world/mesh_animation_player.h"
#include "world/sprite_animation_player.h"
#include "world/types.h"

namespace crown
{
/// Represents a game world.
///
/// @ingroup World
struct World
{
	struct Camera
	{
		UnitId unit;

		ProjectionType::Enum projection_type;

		Frustum frustum;
		f32 fov;
		f32 near_range;
		f32 far_range;
		f32 half_size; // Orthographic projection only.
	};

	u32 _marker;
	Allocator *_allocator;
	ResourceManager *_resource_manager;
	ShaderManager *_shader_manager;
	MaterialManager *_material_manager;
	LuaEnvironment *_lua_environment;
	Pipeline *_pipeline;
	UnitManager *_unit_manager;

	DebugLine *_lines;
	SceneGraph *_scene_graph;
	RenderWorld *_render_world;
	PhysicsWorld *_physics_world;
	SoundWorld *_sound_world;
	ScriptWorld *_script_world;
	SpriteAnimationPlayer *_sprite_animation_player;
	MeshAnimationPlayer *_mesh_animation_player;
	AnimationStateMachine *_animation_state_machine;

	Array<UnitId> _units;
	Array<Camera> _camera;
	HashMap<UnitId, u32> _camera_map;

	EventStream _events;
	Array<UnitId> _changed_units;
	Array<Matrix4x4> _changed_world;
	GuiBuffer _gui_buffer;

	ListNode _guis;
	ListNode _levels;

	ListNode _node;
	UnitId _skydome_unit;

	UnitDestroyCallback _unit_destroy_callback;

#if CROWN_CAN_RELOAD
	Array<const UnitResource *> _unit_resources;
#endif

	CameraInstance camera_make_instance(u32 i)
	{
		CameraInstance inst = { i }; return inst;
	}

	///
	World(Allocator &a
		, ResourceManager &rm
		, ShaderManager &sm
		, MaterialManager &mm
		, UnitManager &um
		, LuaEnvironment &env
		, Pipeline &pl
		);

	///
	~World();

	/// Spawns a new instance of the unit @a name.
	/// Use @a flags to override the instance's @a position, @a rotation and @a scale.
	UnitId spawn_unit(const UnitResource *ur
		, u32 flags
		, const Vector3 &pos
		, const Quaternion &rot
		, const Vector3 &scl
		);

	/// @copydoc World::spawn_unit().
	UnitId spawn_unit(StringId64 name
		, u32 flags = SpawnFlags::NONE
		, const Vector3 &pos = VECTOR3_ZERO
		, const Quaternion &rot = QUATERNION_IDENTITY
		, const Vector3 &scl = VECTOR3_ONE
		);

	/// Spawns a new empty unit and returns its id.
	UnitId spawn_empty_unit();

	/// Spawns a new skydome.
	UnitId spawn_skydome(StringId64 skydome_name);

	/// Destroys the unit with the given @a id.
	void destroy_unit(UnitId unit);

	/// Returns the number of units in the world.
	u32 num_units() const;

	/// Returns all the the units in the world.
	void units(Array<UnitId> &units) const;

	/// Returns the unit with the given Level Editor @a name or UNIT_INVALID if no such unit exists.
	/// If there are multiple units with the same name, a random one will be returned.
	UnitId unit_by_name(StringId32 name);

	///
	void camera_create_instances(const void *components_data, u32 num, const UnitId *unit_lookup, const u32 *unit_index);

	/// Creates a new camera.
	CameraInstance camera_create(UnitId unit, const CameraDesc &cd);

	/// Destroys the camera instance @a i.
	void camera_destroy(CameraInstance camera);

	/// Returns the ID of the camera owned by the *unit*.
	CameraInstance camera_instance(UnitId unit);

	/// Sets the projection type of the @a camera.
	void camera_set_projection_type(CameraInstance camera, ProjectionType::Enum type);

	/// Returns the projection type of the @a camera.
	ProjectionType::Enum camera_projection_type(CameraInstance camera);

	/// Returns the projection matrix of the @a camera.
	Matrix4x4 camera_projection_matrix(CameraInstance camera, f32 aspect_ratio, ProjectionType::Enum projection_type = ProjectionType::COUNT);

	/// Returns the view matrix of the @a camera.
	Matrix4x4 camera_view_matrix(CameraInstance camera);

	/// Returns the field-of-view of the @a camera in degrees.
	f32 camera_fov(CameraInstance camera);

	/// Sets the field-of-view of the @a camera in degrees.
	void camera_set_fov(CameraInstance camera, f32 fov);

	/// Returns the near clip distance of the @a camera.
	f32 camera_near_clip_distance(CameraInstance camera);

	/// Sets the near clip distance of the @a camera.
	void camera_set_near_clip_distance(CameraInstance camera, f32 near);

	/// Returns the far clip distance of the @a camera.
	f32 camera_far_clip_distance(CameraInstance camera);

	/// Sets the far clip distance of the @a camera.
	void camera_set_far_clip_distance(CameraInstance camera, f32 far);

	/// Sets the vertical *half_size* of the orthographic view volume.
	/// The horizontal size is proportional to the viewport's aspect ratio.
	void camera_set_orthographic_size(CameraInstance camera, f32 half_size);

	/// Returns @a pos from screen-space to world-space coordinates.
	Vector3 camera_screen_to_world(CameraInstance camera, const Vector3 &pos);

	/// Returns @a pos from world-space to screen-space coordinates.
	Vector3 camera_world_to_screen(CameraInstance camera, const Vector3 &pos);

	/// Update all animations with @a dt.
	void update_animations(f32 dt);

	/// Update scene with @a dt.
	void update_scene(f32 dt);

	/// Updates all units and sub-systems with the given @a dt delta time.
	void update(f32 dt);

	/// Renders the world using @a view.
	void render(const Matrix4x4 &view, const Matrix4x4 &proj, const Matrix4x4 &persp);

	/// @copydoc SoundWorld::play().
	SoundInstanceId play_sound(StringId64 name, const bool loop, const f32 volume, const f32 range, u32 flags, const Vector3 &pos, StringId32 group);

	/// @copydoc SoundWorld::stop().
	void stop_sound(SoundInstanceId id);

	/// Links the sound @a id to the @a node of the given @a unit.
	/// After this call, the sound @a id will follow the unit @a unit.
	void link_sound(SoundInstanceId id, UnitId unit, s32 node);

	/// Sets the @a pose of the listener.
	void set_listener_pose(const Matrix4x4 &pose);

	/// Sets the @a position of the sound @a id.
	void set_sound_position(SoundInstanceId id, const Vector3 &position);

	/// Sets the @a range of the sound @a id.
	void set_sound_range(SoundInstanceId id, f32 range);

	/// Sets the @a volume of the sound @a id.
	void set_sound_volume(SoundInstanceId id, f32 volume);

	/// Creates a new DebugLine. @a depth_test controls whether to
	/// enable depth test when rendering the lines.
	DebugLine *create_debug_line(bool depth_test);

	/// Destroys the debug @a line.
	void destroy_debug_line(DebugLine &line);

	/// Creates a new screen-space Gui.
	Gui *create_screen_gui();

	/// Creates a new world-space Gui.
	Gui *create_world_gui();

	/// Destroys the @a gui.
	void destroy_gui(Gui &gui);

	/// Loads the level @a name into the world.
	Level *load_level(StringId64 name
		, u32 flags = SpawnFlags::NONE
		, const Vector3 &pos = VECTOR3_ZERO
		, const Quaternion &rot = QUATERNION_IDENTITY
		);

	/// Destroys a @a level previously loaded by load_level().
	void destroy_level(Level &level);

	///
	void post_unit_spawned_events(UnitId *units, u32 num);

	///
	void post_unit_destroyed_event(UnitId unit);

	///
	void post_level_loaded_event();

	/// After this call, units will no longer receive spawned(), unspawned() or any
	/// other callback. This is used only by the editor to prevent user logic to
	/// interfere with editor's assumptions.
	void disable_unit_callbacks();

	///
	void reload_materials(const MaterialResource *old_resource, const MaterialResource *new_resource);

	///
	void reload_units(const UnitResource *old_unit, const UnitResource *new_unit);

	///
	void unit_destroyed_callback(UnitId unit);

	///
	void remove_dead_units();
};

} // namespace crown
