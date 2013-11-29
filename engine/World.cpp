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

#include "Assert.h"
#include "World.h"
#include "Allocator.h"
#include "Device.h"
#include "ResourceManager.h"
#include "SoundRenderer.h"
#include "SoundResource.h"

namespace crown
{

//-----------------------------------------------------------------------------
World::World()
	: m_unit_pool(default_allocator(), MAX_UNITS, sizeof(Unit), CE_ALIGNOF(Unit))
	, m_camera_pool(default_allocator(), MAX_CAMERAS, sizeof(Camera), CE_ALIGNOF(Camera))
	, m_unit_to_camera(default_allocator())
	, m_unit_to_sound(default_allocator())
	, m_unit_to_sprite(default_allocator())
{
}

//-----------------------------------------------------------------------------
UnitId World::spawn_unit(const char* name, const Vector3& pos, const Quaternion& rot)
{
	// Allocate memory for unit
	Unit* unit = CE_NEW(m_unit_pool, Unit)();

	// Fetch resource
	UnitResource* ur = (UnitResource*) device()->resource_manager()->lookup(UNIT_EXTENSION, name);

	// Create Id for the unit
	const UnitId unit_id = m_units.create(unit);
	unit->create(*this, m_graph_manager, ur, unit_id, pos, rot);

	return unit_id;
}

//-----------------------------------------------------------------------------
void World::destroy_unit(UnitId id)
{
	CE_ASSERT(m_units.has(id), "Unit does not exist");

	Unit* unit = m_units.lookup(id);

	unit->destroy();
	CE_DELETE(m_unit_pool, unit);
	m_units.destroy(id);
}

//-----------------------------------------------------------------------------
void World::destroy_unit(Unit* unit)
{
	CE_ASSERT_NOT_NULL(unit);

	destroy_unit(unit->m_id);
}

//-----------------------------------------------------------------------------
void World::link_unit(UnitId child, UnitId parent, int32_t node)
{
	CE_ASSERT(m_units.has(child), "Child unit does not exist");
	CE_ASSERT(m_units.has(parent), "Parent unit does not exist");

	Unit* child_unit = lookup_unit(child);
	Unit* parent_unit = lookup_unit(parent);

	parent_unit->link_node(child_unit->m_root_node, node);
}

//-----------------------------------------------------------------------------
void World::unlink_unit(UnitId child)
{
	CE_ASSERT(m_units.has(child), "Child unit does not exist");
}

//-----------------------------------------------------------------------------
void World::link_camera(CameraId camera, UnitId unit, int32_t node)
{
	UnitToCamera* utc = NULL;

	for (uint32_t i = 0; i < m_unit_to_camera.size(); i++)
	{
		if (m_unit_to_camera[i].camera == camera && m_unit_to_camera[i].unit == unit)
		{
			utc = &m_unit_to_camera[i];
		}
	}

	if (utc != NULL)
	{
		utc->node = node;
	}
	else
	{
		UnitToCamera new_utc;
		new_utc.camera = camera;
		new_utc.unit = unit;
		new_utc.node = node;
		m_unit_to_camera.push_back(new_utc);
	}
}

//-----------------------------------------------------------------------------
void World::unlink_camera(CameraId camera)
{
	(void)camera;
}

//-----------------------------------------------------------------------------
void World::link_sprite(SpriteId sprite, UnitId unit, int32_t node)
{
	UnitToSprite* uts = NULL;

	for (uint32_t i = 0; i < m_unit_to_sprite.size(); i++)
	{
		if (m_unit_to_sprite[i].sprite == sprite && m_unit_to_sprite[i].unit == unit)
		{
			uts = &m_unit_to_sprite[i];
		}
	}

	if (uts != NULL)
	{
		uts->node = node;
	}
	else
	{
		UnitToSprite new_uts;
		new_uts.sprite = sprite;
		new_uts.unit = unit;
		new_uts.node = node;
		m_unit_to_sprite.push_back(new_uts);
	}	
}

//-----------------------------------------------------------------------------
void World::unlink_sprite(SpriteId sprite)
{
	(void)sprite;
}

//-----------------------------------------------------------------------------
Unit* World::lookup_unit(UnitId unit)
{
	CE_ASSERT(m_units.has(unit), "Unit does not exist");

	return m_units.lookup(unit);
}

//-----------------------------------------------------------------------------
Camera* World::lookup_camera(CameraId camera)
{
	CE_ASSERT(m_camera.has(camera), "Camera does not exist");

	return m_camera.lookup(camera);
}

//-----------------------------------------------------------------------------
Mesh* World::lookup_mesh(MeshId mesh)
{
	return m_render_world.lookup_mesh(mesh);
}

//-----------------------------------------------------------------------------
Sprite* World::lookup_sprite(SpriteId sprite)
{
	return m_render_world.lookup_sprite(sprite);
}

//-----------------------------------------------------------------------------
void World::update(Camera& camera, float dt)
{
	// Update scene graphs
	m_graph_manager.update();

	// Update camera poses
	for (uint32_t i = 0; i < m_unit_to_camera.size(); i++)
	{
		const UnitToCamera& utc = m_unit_to_camera[i];

		Camera* cam = m_camera.lookup(utc.camera);
		Unit* unit = m_units.lookup(utc.unit);

		cam->m_world_pose = unit->m_scene_graph->world_pose(utc.node);
	}

	// Updates sound poses
	for (uint32_t i = 0; i < m_unit_to_sound.size(); i++)
	{
		const UnitToSound& uts = m_unit_to_sound[i];

		Sound& sound = m_sounds.lookup(uts.sound);
		Unit* unit = m_units.lookup(uts.unit);

		sound.world = unit->m_scene_graph->world_pose(uts.node);
	}

	// Update sprites poses
	for (uint32_t i = 0; i < m_unit_to_sprite.size(); i++)
	{
		const UnitToSprite& uts = m_unit_to_sprite[i];

		Unit* unit = lookup_unit(uts.unit);
		Sprite* sprite = lookup_sprite(uts.sprite);

		sprite->m_world_pose = unit->m_scene_graph->world_pose(uts.node);
	}

	// Update render world
	m_render_world.update(camera.m_world_pose, camera.m_projection, camera.m_view_x, camera.m_view_y,
							camera.m_view_width, camera.m_view_height, dt);
}

//-----------------------------------------------------------------------------
RenderWorld& World::render_world()
{
	return m_render_world;
}

//-----------------------------------------------------------------------------
CameraId World::create_camera(int32_t node, const Vector3& pos, const Quaternion& rot)
{
	// Allocate memory for camera
	Camera* camera = CE_NEW(m_camera_pool, Camera)();

	// Create Id for the camera
	const CameraId camera_id = m_camera.create(camera);
	camera->create(node, pos, rot);

	return camera_id;
}

//-----------------------------------------------------------------------------
void World::destroy_camera(CameraId id)
{
	CE_ASSERT(m_camera.has(id), "Camera does not exist");

	Camera* camera = m_camera.lookup(id);
	CE_DELETE(m_camera_pool, camera);
	m_camera.destroy(id);
}

//-----------------------------------------------------------------------------
MeshId World::create_mesh(ResourceId id, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	return m_render_world.create_mesh(id, node, pos, rot);
}

//-----------------------------------------------------------------------------
void World::destroy_mesh(MeshId id)
{
	m_render_world.destroy_mesh(id);
}

//-----------------------------------------------------------------------------
SpriteId World::create_sprite(ResourceId id, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	return m_render_world.create_sprite(id, node, pos, rot);
}

//-----------------------------------------------------------------------------
void World::destroy_sprite(SpriteId id)
{
	m_render_world.destroy_sprite(id);
}

//-----------------------------------------------------------------------------
SoundId World::play_sound(const char* name, const bool loop, const float volume, const Vector3& pos, const float range)
{
	SoundResource* sound = (SoundResource*)device()->resource_manager()->lookup(SOUND_EXTENSION, name);
	SoundRenderer* sr = device()->sound_renderer();

	const SoundSourceId source = sr->create_sound_source();

	Sound s;
	s.buffer = sound->m_id;
	s.source = source;
	s.world = Matrix4x4(Quaternion::IDENTITY, pos);
	s.volume = volume;
	s.range = range;
	s.loop = loop;
	s.playing = false;

	SoundId id = m_sounds.create(s);

	sr->bind_buffer(s.buffer, s.source);
	sr->set_sound_loop(s.source, s.loop);
	sr->set_sound_gain(s.source, s.volume);
	sr->set_sound_max_distance(s.source, s.range);
	sr->set_sound_position(s.source, s.world.translation());
	sr->play_sound(s.source);

	return id;
}

//-----------------------------------------------------------------------------
void World::stop_sound(SoundId id)
{
	CE_ASSERT(m_sounds.has(id), "Sound does not exists");

	const Sound& s = m_sounds.lookup(id);
	SoundRenderer* sr = device()->sound_renderer();

	sr->pause_sound(s.source);
	sr->unbind_buffer(s.source);
	sr->destroy_sound_source(s.source);

	m_sounds.destroy(id);
}

//-----------------------------------------------------------------------------
void World::link_sound(SoundId id, Unit* unit, int32_t node)
{
	//CE_ASSERT(m_units.has(unit), "Unit does not exists");
	CE_ASSERT(m_sounds.has(id), "Sound does not exists");

	UnitToSound uts;
	uts.sound = id;
	uts.unit = unit->m_id;
	uts.node = node;

	m_unit_to_sound.push_back(uts);
}

//-----------------------------------------------------------------------------
void World::set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at)
{
	device()->sound_renderer()->set_listener(pos, vel, or_up, or_at);
}

//-----------------------------------------------------------------------------
void World::set_sound_position(SoundId id, const Vector3& pos)
{
	CE_ASSERT(m_sounds.has(id), "Sound does not exists");

	Sound& sound = m_sounds.lookup(id);
	sound.world = Matrix4x4(Quaternion::IDENTITY, pos);
}

//-----------------------------------------------------------------------------
void World::set_sound_range(SoundId id, const float range)
{
	CE_ASSERT(m_sounds.has(id), "Sound does not exists");

	Sound& sound = m_sounds.lookup(id);
	sound.range = range;
}

//-----------------------------------------------------------------------------
void World::set_sound_volume(SoundId id, const float vol)
{
	CE_ASSERT(m_sounds.has(id), "Sound does not exists");

	Sound& sound = m_sounds.lookup(id);
	sound.volume = vol;
}

} // namespace crown
