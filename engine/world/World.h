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

#include "Camera.h"
#include "IdArray.h"
#include "LinearAllocator.h"
#include "PhysicsTypes.h"
#include "PhysicsWorld.h"
#include "PoolAllocator.h"
#include "RenderWorld.h"
#include "RenderWorldTypes.h"
#include "SceneGraphManager.h"
#include "Types.h"
#include "Unit.h"
#include "Vector.h"
#include "WorldTypes.h"
#include "SoundWorld.h"
#include "EventStream.h"

namespace crown
{

struct Mesh;
struct Sprite;
struct Actor;
struct Vector3;
struct Quaternion;
struct PhysicsResource;
struct DebugLine;

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
	UnitId spawn_unit(const ResourceId id, UnitResource* ur, const Vector3& pos, const Quaternion& rot);

	/// Destroys the unit with the given @a id.
	void destroy_unit(UnitId id);
	void reload_units(UnitResource* old_ur, UnitResource* new_ur);

	/// Returns the number of units in the world.
	uint32_t num_units() const;

	/// Returns all the the units in the world.
	void units(Array<UnitId>& units) const;

	/// Links the unit @a child to the @a node of the unit @a parent.
	/// After this call, @a child will follow the @a parent unit.
	void link_unit(UnitId child, UnitId parent, int32_t node);

	/// Unlinks the unit @a unit from its parent if it has any.
	void unlink_unit(UnitId unit);

	Unit* lookup_unit(UnitId unit);
	Camera* lookup_camera(CameraId camera);

	/// Updates all units and sub-systems with the given @a dt delta time.
	void update(float dt);

	/// Renders the world form the point of view of the given @a camera.
	void render(Camera* camera);

	CameraId create_camera(SceneGraph& sg, int32_t node, ProjectionType::Enum type, float near, float far);
	void destroy_camera(CameraId id);

	/// Plays the sound with the given @æ name at the given @a position, with the given
	/// @a volume and @a range. @a loop controls whether the sound must loop or not.
	SoundInstanceId play_sound(const char* name, bool loop = false, float volume = 1.0f, const Vector3& position = vector3::ZERO, float range = 50.0f);

	/// Stops the sound with the given @a id.
	void stop_sound(SoundInstanceId id);

	/// Links the sound @a if to the @a node of the given @æ unit.
	/// After this call, the sound @a id will follow the unit @æ unit.
	void link_sound(SoundInstanceId id, Unit* unit, int32_t node);

	/// Sets the @a pose of the listener.
	void set_listener_pose(const Matrix4x4& pose);

	/// Sets the @a position of the sound @a id.
	void set_sound_position(SoundInstanceId id, const Vector3& position);

	/// Sets the @a range of the sound @a id.
	void set_sound_range(SoundInstanceId id, float range);

	/// Sets the @a volume of the sound @a id.
	void set_sound_volume(SoundInstanceId id, float volume);

	/// Creates a new window-space Gui of size @width and @a height.
	GuiId create_window_gui(uint16_t width, uint16_t height);

	/// Destroys the gui with the given @a id.
	void destroy_gui(GuiId id);
	Gui* lookup_gui(GuiId id);

	DebugLine* create_debug_line(bool depth_test);
	void destroy_debug_line(DebugLine* line);

	SceneGraphManager* scene_graph_manager();
	RenderWorld* render_world();
	PhysicsWorld* physics_world();
	SoundWorld* sound_world();

private:

	void process_physics_events();

private:

	PoolAllocator						m_unit_pool;
	PoolAllocator						m_camera_pool;

	IdArray<CE_MAX_UNITS, Unit*>		m_units;
	IdArray<CE_MAX_CAMERAS, Camera*>	m_cameras;

	SceneGraphManager					m_scenegraph_manager;
	RenderWorld							m_render_world;
	PhysicsWorld						m_physics_world;
	SoundWorld*							m_sound_world;

	WorldId								m_id;

	EventStream							m_events;
};

} // namespace crown
