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

class World
{
public:
										World();
										~World();

	WorldId								id() const;
	void								set_id(WorldId id);

	UnitId								spawn_unit(const char* name, const Vector3& pos = vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	UnitId								spawn_unit(const char* name, UnitResource* ur, const Vector3& pos, const Quaternion& rot);
	void								destroy_unit(UnitId id);
	void								reload_units(UnitResource* old_ur, UnitResource* new_ur);

	uint32_t							num_units() const;

	void								link_unit(UnitId child, UnitId parent, int32_t node);
	void								unlink_unit(UnitId unit);

	Unit*								lookup_unit(UnitId unit);
	Camera*								lookup_camera(CameraId camera);

	void								update(float dt);
	void								render(Camera* camera);

	CameraId							create_camera(SceneGraph& sg, int32_t node);
	void								destroy_camera(CameraId id);

	SoundInstanceId						play_sound(const char* name, bool loop = false, float volume = 1.0f, const Vector3& pos = vector3::ZERO, float range = 50.0f);
	void								stop_sound(SoundInstanceId sound);
	void								link_sound(SoundInstanceId sound, Unit* unit, int32_t node);
	void								set_listener_pose(const Matrix4x4& pose);
	void								set_sound_position(SoundInstanceId sound, const Vector3& pos);
	void								set_sound_range(SoundInstanceId sound, float range);
	void								set_sound_volume(SoundInstanceId sound, float vol);

	GuiId								create_window_gui(const char* name);
	GuiId								create_world_gui(const Matrix4x4 pose, const uint32_t width, const uint32_t height);
	void								destroy_gui(GuiId id);
	Gui*								lookup_gui(GuiId id);

	DebugLine*							create_debug_line(bool depth_test);
	void								destroy_debug_line(DebugLine* line);

	SceneGraphManager*					scene_graph_manager();
	RenderWorld*						render_world();
	PhysicsWorld*						physics_world();
	SoundWorld*							sound_world();

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
