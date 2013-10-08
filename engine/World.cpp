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
UnitId World::spawn_unit(const char* /*name*/, const Vec3& pos, const Quat& rot)
{
	const UnitId unit = m_unit_table.create();

	m_units[unit.index].create(*this, pos, rot);

	// m_units[unit.index].load(ur);

	return unit;
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

	Unit& child_unit = m_units[child.index];
	Unit& parent_unit =  m_units[parent.index];

	parent_unit.m_scene_graph.link(child_unit.m_root_node, parent_unit.m_root_node);
}

//-----------------------------------------------------------------------------
void World::unlink_unit(UnitId child, UnitId parent)
{
	CE_ASSERT(m_unit_table.has(child), "Child unit does not exist");
	CE_ASSERT(m_unit_table.has(parent), "Parent unit does not exist");

	Unit& child_unit = m_units[child.index];
	Unit& parent_unit =  m_units[parent.index];

	parent_unit.m_scene_graph.unlink(child_unit.m_root_node);		
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
void World::update(Camera& camera, float dt)
{
	m_render_world.update(camera, dt);
}

//-----------------------------------------------------------------------------
RenderWorld& World::render_world()
{
	return m_render_world;
}

//-----------------------------------------------------------------------------
CameraId World::create_camera(int32_t node, const Vec3& pos, const Quat& rot)
{
	CameraId camera = m_camera_table.create();

	m_camera[camera.index].create(node, pos, rot);
	return camera;
}

//-----------------------------------------------------------------------------
void World::destroy_camera(CameraId camera)
{
	m_camera_table.destroy(camera);
}

//-----------------------------------------------------------------------------
Mesh* World::mesh()
{
	return m_render_world.mesh();
}

//-----------------------------------------------------------------------------
SoundInstanceId World::play_sound(const char* name, const bool loop, const float volume, const Vec3& pos, const float range)
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

	Vec3 pos = m_units[unit.index].world_position();
	device()->sound_renderer()->set_sound_position(m_sound[sound.index].m_sound, pos);
}

//-----------------------------------------------------------------------------
void World::set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at)
{
	device()->sound_renderer()->set_listener(pos, vel, or_up, or_at);
}

//-----------------------------------------------------------------------------
void World::set_sound_position(SoundInstanceId sound, const Vec3& pos)
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
