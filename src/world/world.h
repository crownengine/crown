/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "event_stream.h"
#include "lua_types.h"
#include "math_types.h"
#include "resource_types.h"
#include "string_id.h"
#include "types.h"
#include "world_types.h"

namespace crown
{
/// Represents a game world.
///
/// @ingroup World
class World
{
public:

	World(Allocator& a, ResourceManager& rm, ShaderManager& sm, MaterialManager& mm, UnitManager& um, LuaEnvironment& env);
	~World();

	UnitId spawn_unit(const UnitResource& ur, const Vector3& position = VECTOR3_ZERO, const Quaternion& rotation = QUATERNION_IDENTITY);

	/// Spawns a new instance of the unit @a name at the given @a position and @a rotation.
	UnitId spawn_unit(StringId64 name, const Vector3& pos, const Quaternion& rot);

	/// Spawns a new empty unit with the given @a id.
	void spawn_empty_unit(UnitId id);

	/// Destroys the unit with the given @a id.
	void destroy_unit(UnitId id);

	/// Returns the number of units in the world.
	uint32_t num_units() const;

	/// Returns all the the units in the world.
	void units(Array<UnitId>& units) const;

	/// Creates a new camera.
	CameraInstance create_camera(UnitId id, const CameraDesc& cd);

	/// Destroys the camera @a id.
	void destroy_camera(CameraInstance i);

	/// Returns the camera owned by unit @a id.
	CameraInstance camera(UnitId id);

	/// Sets the projection type of the camera.
	void set_camera_projection_type(CameraInstance i, ProjectionType::Enum type);

	/// Returns the projection type of the camera.
	ProjectionType::Enum camera_projection_type(CameraInstance i) const;

	/// Returns the projection matrix of the camera.
	const Matrix4x4& camera_projection_matrix(CameraInstance i) const;

	/// Returns the view matrix of the camera.
	Matrix4x4 camera_view_matrix(CameraInstance i) const;

	/// Returns the field-of-view of the camera in degrees.
	float camera_fov(CameraInstance i) const;

	/// Sets the field-of-view of the camera in degrees.
	void set_camera_fov(CameraInstance i, float fov);

	/// Returns the aspect ratio of the camera. (Perspective projection only.)
	float camera_aspect(CameraInstance i) const;

	/// Sets the aspect ratio of the camera. (Perspective projection only.)
	void set_camera_aspect(CameraInstance i, float aspect);

	/// Returns the near clip distance of the camera.
	float camera_near_clip_distance(CameraInstance i) const;

	/// Sets the near clip distance of the camera.
	void set_camera_near_clip_distance(CameraInstance i, float near);

	/// Returns the far clip distance of the camera.
	float camera_far_clip_distance(CameraInstance i) const;

	/// Sets the far clip distance of the camera.
	void set_camera_far_clip_distance(CameraInstance i, float far);

	/// Sets the coordinates for orthographic clipping planes. (Orthographic projection only.)
	void set_camera_orthographic_metrics(CameraInstance i, float left, float right, float bottom, float top);

	/// Sets the coordinates for the camera viewport in pixels.
	void set_camera_viewport_metrics(CameraInstance i, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

	/// Returns @a pos from screen-space to world-space coordinates.
	Vector3 camera_screen_to_world(CameraInstance i, const Vector3& pos);

	/// Returns @a pos from world-space to screen-space coordinates.
	Vector3 camera_world_to_screen(CameraInstance i, const Vector3& pos);

	/// Update all animations with @a dt.
	void update_animations(float dt);

	/// Update scene with @a dt.
	void update_scene(float dt);

	/// Updates all units and sub-systems with the given @a dt delta time.
	void update(float dt);

	/// Renders the world form the point of view of camera @a i.
	void render(CameraInstance i);

	SoundInstanceId play_sound(const SoundResource& sr, bool loop = false, float volume = 1.0f, const Vector3& position = VECTOR3_ZERO, float range = 50.0f);

	/// Plays the sound with the given @a name at the given @a position, with the given
	/// @a volume and @a range. @a loop controls whether the sound must loop or not.
	SoundInstanceId play_sound(StringId64 name, const bool loop, const float volume, const Vector3& pos, const float range);

	/// Stops the sound with the given @a id.
	void stop_sound(SoundInstanceId id);

	/// Links the sound @a id to the @a node of the given @a unit.
	/// After this call, the sound @a id will follow the unit @a unit.
	void link_sound(SoundInstanceId id, UnitId unit, int32_t node);

	/// Sets the @a pose of the listener.
	void set_listener_pose(const Matrix4x4& pose);

	/// Sets the @a position of the sound @a id.
	void set_sound_position(SoundInstanceId id, const Vector3& position);

	/// Sets the @a range of the sound @a id.
	void set_sound_range(SoundInstanceId id, float range);

	/// Sets the @a volume of the sound @a id.
	void set_sound_volume(SoundInstanceId id, float volume);

	/// Creates a new DebugLine. @a depth_test controls whether to
	/// enable depth test when rendering the lines.
	DebugLine* create_debug_line(bool depth_test);

	/// Destroys the debug @a line.
	void destroy_debug_line(DebugLine& line);

	/// Loads the level @a name into the world.
	Level* load_level(const LevelResource& lr, const Vector3& pos, const Quaternion& rot);
	Level* load_level(StringId64 name, const Vector3& pos, const Quaternion& rot);

	/// Returns the events.
	EventStream& events();

	/// Returns the scene graph.
	SceneGraph* scene_graph();

	/// Returns the rendering sub-world.
	RenderWorld* render_world();

	/// Returns the physics sub-world.
	PhysicsWorld* physics_world();

	/// Returns the sound sub-world.
	SoundWorld* sound_world();

	static const uint32_t MARKER = 0xfb6ce2d3;

private:

	CameraInstance make_camera_instance(uint32_t i) { CameraInstance inst = { i }; return inst; }

	void post_unit_spawned_event(UnitId id);
	void post_unit_destroyed_event(UnitId id);
	void post_level_loaded_event();

private:

	struct Camera
	{
		UnitId unit;

		ProjectionType::Enum projection_type;
		Matrix4x4 projection;

		Frustum frustum;
		float fov;
		float aspect;
		float near;
		float far;

		// Orthographic projection only
		float left;
		float right;
		float bottom;
		float top;

		uint16_t view_x;
		uint16_t view_y;
		uint16_t view_width;
		uint16_t view_height;

		void update_projection_matrix();
	};

	uint32_t _marker;

	Allocator* _allocator;
	ResourceManager* _resource_manager;
	LuaEnvironment* _lua_environment;
	UnitManager* _unit_manager;

	DebugLine* _lines;
	SceneGraph* _scene_graph;
	RenderWorld* _render_world;
	PhysicsWorld* _physics_world;
	SoundWorld* _sound_world;

	Array<UnitId> _units;
	Array<Level*> _levels;
	Array<Camera> _camera;
	Hash<uint32_t> _camera_map;

	EventStream _events;
};

} // namespace crown
