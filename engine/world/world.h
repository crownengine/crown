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

#include "camera.h"
#include "id_array.h"
#include "linear_allocator.h"
#include "physics_types.h"
#include "physics_world.h"
#include "pool_allocator.h"
#include "render_world.h"
#include "render_world_types.h"
#include "scene_graph_manager.h"
#include "types.h"
#include "unit.h"
#include "vector.h"
#include "world_types.h"
#include "sound_world.h"
#include "event_stream.h"
#include "sprite_animation_player.h"
#include "resource_types.h"

namespace crown
{

/// @defgroup World World

/// Represents a game world.
///
/// @ingroup World
class World
{
public:

	World();
	~World();

	WorldId id() const;
	void set_id(WorldId id);

	/// Spawns a new instance of the unit @a name at the given @a position and @a rotation.
	UnitId spawn_unit(const char* name, const Vector3& position = vector3::ZERO, const Quaternion& rotation = quaternion::IDENTITY);
	UnitId spawn_unit(StringId64 name, const Vector3& pos, const Quaternion& rot);
	UnitId spawn_unit(ResourceId id, const Vector3& pos, const Quaternion& rot);
	UnitId spawn_unit(const ResourceId id, UnitResource* ur, const Vector3& pos, const Quaternion& rot);

	/// Destroys the unit with the given @a id.
	void destroy_unit(UnitId id);

	/// Reloads all the units with the associated resource @a old_ur.
	void reload_units(UnitResource* old_ur, UnitResource* new_ur);

	/// Returns the number of units in the world.
	uint32_t num_units() const;

	/// Returns all the the units in the world.
	void units(Array<UnitId>& units) const;

	/// Links the unit @a child to the @a node of the unit @a parent.
	/// After this call, @a child will follow the @a parent unit.
	void link_unit(UnitId child, UnitId parent, int32_t node);

	/// Unlinks the unit @a id from its parent if it has any.
	void unlink_unit(UnitId id);

	/// Returns the unit @a id.
	Unit* get_unit(UnitId id);

	/// Returns the camera @a id.
	Camera* get_camera(CameraId id);

	/// Update all animations with @a dt.
	void update_animations(float dt);

	/// Update scene with @a dt.
	void update_scene(float dt);

	/// Updates all units and sub-systems with the given @a dt delta time.
	void update(float dt);

	/// Renders the world form the point of view of the given @a camera.
	void render(Camera* camera);

	CameraId create_camera(SceneGraph& sg, int32_t node, ProjectionType::Enum type, float near, float far);

	/// Destroys the camera @a id.
	void destroy_camera(CameraId id);

	/// Plays the sound with the given @a name at the given @a position, with the given
	/// @a volume and @a range. @a loop controls whether the sound must loop or not.
	SoundInstanceId play_sound(const char* name, bool loop = false, float volume = 1.0f, const Vector3& position = vector3::ZERO, float range = 50.0f);
	SoundInstanceId play_sound(StringId64 name, const bool loop, const float volume, const Vector3& pos, const float range);
	SoundInstanceId play_sound(ResourceId id, const bool loop, const float volume, const Vector3& pos, const float range);
	SoundInstanceId play_sound(SoundResource* sr, const bool loop, const float volume, const Vector3& pos, const float range);

	/// Stops the sound with the given @a id.
	void stop_sound(SoundInstanceId id);

	/// Links the sound @a id to the @a node of the given @a unit.
	/// After this call, the sound @a id will follow the unit @a unit.
	void link_sound(SoundInstanceId id, Unit* unit, int32_t node);

	/// Sets the @a pose of the listener.
	void set_listener_pose(const Matrix4x4& pose);

	/// Sets the @a position of the sound @a id.
	void set_sound_position(SoundInstanceId id, const Vector3& position);

	/// Sets the @a range of the sound @a id.
	void set_sound_range(SoundInstanceId id, float range);

	/// Sets the @a volume of the sound @a id.
	void set_sound_volume(SoundInstanceId id, float volume);

	/// Creates a new window-space Gui of size @a width and @a height.
	GuiId create_window_gui(uint16_t width, uint16_t height, const char* material);

	/// Destroys the gui with the given @a id.
	void destroy_gui(GuiId id);

	/// Returns the gui @a id.
	Gui* get_gui(GuiId id);

	/// Creates a new DebugLine. @a depth_test controls whether to
	/// enable depth test when rendering the lines.
	DebugLine* create_debug_line(bool depth_test);

	/// Destroys the debug @a line.
	void destroy_debug_line(DebugLine* line);

	/// Loads the level @a name into the world.
	void load_level(const char* name);
	void load_level(const LevelResource* lr);

	SceneGraphManager* scene_graph_manager();
	SpriteAnimationPlayer* sprite_animation_player();

	/// Returns the rendering sub-world.
	RenderWorld* render_world();

	/// Returns the physics sub-world.
	PhysicsWorld* physics_world();

	/// Returns the sound sub-world.
	SoundWorld* sound_world();

private:

	void post_unit_spawned_event(UnitId id);
	void post_unit_destroyed_event(UnitId id);
	void post_level_loaded_event();
	void process_physics_events();

private:

	PoolAllocator m_unit_pool;
	PoolAllocator m_camera_pool;

	IdArray<CE_MAX_UNITS, Unit*> m_units;
	IdArray<CE_MAX_CAMERAS, Camera*> m_cameras;

	SceneGraphManager m_scenegraph_manager;
	SpriteAnimationPlayer m_sprite_animation_player;
	RenderWorld m_render_world;
	PhysicsWorld m_physics_world;
	SoundWorld* m_sound_world;

	WorldId m_id;

	EventStream m_events;
};

} // namespace crown
