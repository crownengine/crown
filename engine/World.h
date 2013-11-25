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
#include "SoundRenderer.h"
#include "PoolAllocator.h"

namespace crown
{

#define MAX_UNITS 65000
#define	MAX_SOUNDS 64
#define MAX_CAMERAS 16

typedef Id UnitId;
typedef Id CameraId;
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

struct UnitToCamera
{
	UnitId unit;
	int32_t node;
	CameraId camera;
};

struct UnitToSound
{
	UnitId unit;
	SoundId sound;
	int32_t node;
};

struct UnitToSprite
{
	UnitId unit;
	SpriteId sprite;
	int32_t node;
};

class Mesh;
class Sprite;
class Vector3;
class Quaternion;

class World
{
public:
							World();

	UnitId					spawn_unit(const char* name, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion(Vector3(0, 1, 0), 0.0f));
	void					destroy_unit(UnitId unit);
	void					destroy_unit(Unit* unit);

	void					link_unit(UnitId child, UnitId parent, int32_t node);
	void					unlink_unit(UnitId unit);

	void					link_camera(CameraId camera, UnitId unit, int32_t node);
	void					unlink_camera(CameraId camera);

	void					link_sprite(SpriteId sprite, UnitId unit, int32_t node);
	void					unlink_sprite(SpriteId sprite);

	Unit*					lookup_unit(UnitId unit);
	Camera*					lookup_camera(CameraId camera);
	Mesh*					lookup_mesh(MeshId mesh);
	Sprite*					lookup_sprite(SpriteId sprite);

	RenderWorld&			render_world();
	void					update(Camera& camera, float dt);

	CameraId				create_camera(int32_t node, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	void					destroy_camera(CameraId camera);

	MeshId					create_mesh(ResourceId id, int32_t node, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	void					destroy_mesh(MeshId id);

	SpriteId				create_sprite(ResourceId id, int32_t node = -1, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	void					destroy_sprite(SpriteId id);

	SoundId					play_sound(const char* name, const bool loop = false, const float volume = 1.0f, const Vector3& pos = Vector3::ZERO, const float range = 50.0f);
	void					stop_sound(SoundId sound);
	void					link_sound(SoundId sound, Unit* unit, int32_t node);
	void					set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at);
	void					set_sound_position(SoundId sound, const Vector3& pos);
	void					set_sound_range(SoundId sound, const float range);
	void					set_sound_volume(SoundId sound, const float vol);

private:

	PoolAllocator						m_unit_pool;
	PoolAllocator						m_camera_pool;

	IdArray<MAX_UNITS, Unit*>			m_units;
	IdArray<MAX_CAMERAS, Camera*>		m_camera;
	IdArray<MAX_SOUNDS, Sound> 			m_sounds;

	// Connections
	List<UnitToCamera>					m_unit_to_camera;
	List<UnitToSound>					m_unit_to_sound;
	List<UnitToSprite>					m_unit_to_sprite;

	RenderWorld							m_render_world;
};

} // namespace crown
