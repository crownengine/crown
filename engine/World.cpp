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
	: m_allocator(default_allocator(), 1024 * 1024)
	, m_is_init(false)
	, m_units(default_allocator())
	, m_camera(default_allocator())
{
}

//-----------------------------------------------------------------------------
void World::init()
{
}

//-----------------------------------------------------------------------------
void World::shutdown()
{
}

//-----------------------------------------------------------------------------
UnitId World::spawn_unit(const char* /*name*/, const Vector3& pos, const Quaternion& rot)
{
	const UnitId unit_id = m_unit_table.create();

	Unit unit;
	unit.create(*this, m_scene_graph[unit_id.index], m_component[unit_id.index], unit_id, pos, rot);

	// Test stuff
	int32_t cam_node = unit.m_scene_graph->create_node(unit.m_root_node, pos, rot);
	CameraId camera = create_camera(unit_id, cam_node);

	SpriteId sprite = m_render_world.create_sprite("sprites/loading");
	unit.m_component->add_component("camera", ComponentType::CAMERA, camera);
	unit.m_component->add_component("sprite", ComponentType::SPRITE, sprite);

	m_units.push_back(unit);

	return unit_id;
}

//-----------------------------------------------------------------------------
void World::kill_unit(UnitId unit)
{
	CE_ASSERT(m_unit_table.has(unit), "Unit does not exist");
	(void)unit;
}

//-----------------------------------------------------------------------------
void World::link_unit(UnitId child, UnitId parent)
{
	CE_ASSERT(m_unit_table.has(child), "Child unit does not exist");
	CE_ASSERT(m_unit_table.has(parent), "Parent unit does not exist");
}

//-----------------------------------------------------------------------------
void World::unlink_unit(UnitId child, UnitId parent)
{
	CE_ASSERT(m_unit_table.has(child), "Child unit does not exist");
	CE_ASSERT(m_unit_table.has(parent), "Parent unit does not exist");
}

//-----------------------------------------------------------------------------
Unit* World::lookup_unit(UnitId unit)
{
	CE_ASSERT(m_unit_table.has(unit), "Unit does not exist");

	return &m_units[unit.index];
}

//-----------------------------------------------------------------------------
Camera* World::lookup_camera(CameraId camera)
{
	CE_ASSERT(m_camera_table.has(camera), "Camera does not exist");

	return &m_camera[camera.index];
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
	// Feed the scene graph with camera local pose
	for (uint cc = 0; cc < m_camera.size(); cc++)
	{
		Camera& cam = m_camera[cc];
		SceneGraph& graph = m_scene_graph[cam.m_unit.index];

		graph.set_local_pose(cam.m_node, cam.m_local_pose);
	}

	// Update all the units
	for (uint32_t uu = 0; uu < m_units.size(); uu++)
	{
		Unit& unit = m_units[uu];
		SceneGraph& graph = m_scene_graph[unit.m_id.index];

		// Update unit's scene graph
		graph.update();
	}

	// Fetch the camera world poses from scene graph
	for (uint32_t cc = 0; cc < m_camera.size(); cc++)
	{
		Camera& cam = m_camera[cc];
		SceneGraph& graph = m_scene_graph[cam.m_unit.index];

		cam.m_world_pose = graph.world_pose(cam.m_node);
	}

	// Update render world
	m_render_world.update(camera, dt);
}

//-----------------------------------------------------------------------------
RenderWorld& World::render_world()
{
	return m_render_world;
}

//-----------------------------------------------------------------------------
CameraId World::create_camera(UnitId unit, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	CameraId camera_id = m_camera_table.create();

	Camera camera;
	camera.create(unit, node, pos, rot);

	m_camera.push_back(camera);

	return camera_id;
}

//-----------------------------------------------------------------------------
void World::destroy_camera(CameraId camera)
{
	m_camera_table.destroy(camera);
}

//-----------------------------------------------------------------------------
SoundInstanceId World::play_sound(const char* name, const bool loop, const float volume, const Vector3& pos, const float range)
{
	SoundInstanceId id = m_sound_table.create();

	SoundResource* sound = (SoundResource*)device()->resource_manager()->lookup(SOUND_EXTENSION, name);

	m_sound[id.index].m_sound = sound->m_id;

	device()->sound_renderer()->set_sound_loop(m_sound[id.index].m_sound, loop);
	device()->sound_renderer()->set_sound_gain(m_sound[id.index].m_sound, volume);
	device()->sound_renderer()->set_sound_position(m_sound[id.index].m_sound, pos);
	device()->sound_renderer()->set_sound_max_distance(m_sound[id.index].m_sound, range);

	device()->sound_renderer()->play_sound(m_sound[id.index].m_sound);

	return id;
}

//-----------------------------------------------------------------------------
void World::pause_sound(SoundInstanceId sound)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	device()->sound_renderer()->pause_sound(m_sound[sound.index].m_sound);
}

//-----------------------------------------------------------------------------
void World::link_sound(SoundInstanceId sound, UnitId unit)
{
	CE_ASSERT(m_unit_table.has(unit), "Unit does not exists");
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	Vector3 pos = m_units[unit.index].world_position();
	device()->sound_renderer()->set_sound_position(m_sound[sound.index].m_sound, pos);
}

//-----------------------------------------------------------------------------
void World::set_listener(const Vector3& pos, const Vector3& vel, const Vector3& or_up, const Vector3& or_at)
{
	device()->sound_renderer()->set_listener(pos, vel, or_up, or_at);
}

//-----------------------------------------------------------------------------
void World::set_sound_position(SoundInstanceId sound, const Vector3& pos)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	device()->sound_renderer()->set_sound_position(m_sound[sound.index].m_sound, pos);
}

//-----------------------------------------------------------------------------
void World::set_sound_range(SoundInstanceId sound, const float range)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	device()->sound_renderer()->set_sound_max_distance(m_sound[sound.index].m_sound, range);
}

//-----------------------------------------------------------------------------
void World::set_sound_volume(SoundInstanceId sound, const float vol)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	device()->sound_renderer()->set_sound_gain(m_sound[sound.index].m_sound, vol);
}

} // namespace crown
