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

#include "Types.h"
#include "HeapAllocator.h"
#include "IdArray.h"
#include "LinearAllocator.h"
#include "Unit.h"
#include "Camera.h"
#include "Vector.h"
#include "RenderWorld.h"
#include "PhysicsWorld.h"
#include "SoundRenderer.h"
#include "PoolAllocator.h"
#include "SceneGraphManager.h"
#include "PhysicsTypes.h"
#include "WorldTypes.h"

namespace crown
{

#define MAX_UNITS 65000
#define	MAX_SOUNDS 64
#define MAX_CAMERAS 16

typedef Id MeshId;
typedef Id SoundId;
typedef Id SpriteId;

struct Sound
{
	SoundBufferId buffer;
	SoundSourceId source;
	
	Matrix4x4 world;
	float volume;
	float range;
	bool loop : 1;
	bool playing : 1;
};

struct UnitToSound
{
	UnitId unit;
	SoundId sound;
	int32_t node;
};

struct Mesh;
struct Sprite;
struct Actor;
struct Vector3;
struct Quaternion;
struct PhysicsResource;

class World
{
public:
										World();
										~World();

	WorldId								id() const;
	void								set_id(WorldId id);

	UnitId								spawn_unit(const char* name, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	UnitId								spawn_unit(UnitResource* ur, const Vector3& pos, const Quaternion& rot);
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

	SoundId								play_sound(const char* name, bool loop = false, float volume = 1.0f, const Vector3& pos = Vector3::ZERO, float range = 50.0f);
	SoundId								play_sound(SoundResource* sr, bool loop, float volume, const Vector3& pos, float range);
	void								stop_sound(SoundId sound);
	void								link_sound(SoundId sound, Unit* unit, int32_t node);
	void								set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at);
	void								set_sound_position(SoundId sound, const Vector3& pos);
	void								set_sound_range(SoundId sound, const float range);
	void								set_sound_volume(SoundId sound, const float vol);

	SceneGraphManager*					scene_graph_manager();
	RenderWorld*						render_world();
	PhysicsWorld*						physics_world();

private:

	PoolAllocator						m_unit_pool;
	PoolAllocator						m_camera_pool;

	IdArray<MAX_UNITS, Unit*>			m_units;
	IdArray<MAX_CAMERAS, Camera*>		m_cameras;
	IdArray<MAX_SOUNDS, Sound> 			m_sounds;

	// Connections
	List<UnitToSound>					m_unit_to_sound;

	SceneGraphManager					m_scenegraph_manager;
	RenderWorld							m_render_world;
	PhysicsWorld						m_physics_world;

	WorldId								m_id;
};

} // namespace crown
