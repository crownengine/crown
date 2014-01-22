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

namespace crown
{

//-----------------------------------------------------------------------------
World::World()
	: m_unit_pool(default_allocator(), MAX_UNITS, sizeof(Unit), CE_ALIGNOF(Unit))
	, m_camera_pool(default_allocator(), MAX_CAMERAS, sizeof(Camera), CE_ALIGNOF(Camera))
	, m_events(default_allocator())
{
	m_id.id = INVALID_ID;
	m_sound_world = SoundWorld::create(default_allocator());
}

//-----------------------------------------------------------------------------
World::~World()
{
	// Destroy all units
	for (uint32_t i = 0; i < m_units.size(); i++)
	{
		CE_DELETE(m_unit_pool, m_units[i]);
	}

	SoundWorld::destroy(default_allocator(), m_sound_world);
}

//-----------------------------------------------------------------------------
WorldId World::id() const
{
	return m_id;
}

//-----------------------------------------------------------------------------
void World::set_id(WorldId id)
{
	m_id = id;
}

//-----------------------------------------------------------------------------
UnitId World::spawn_unit(const char* name, const Vector3& pos, const Quaternion& rot)
{
	UnitResource* ur = (UnitResource*) device()->resource_manager()->lookup(UNIT_EXTENSION, name);
	return spawn_unit(ur, pos, rot);
}

//-----------------------------------------------------------------------------
UnitId World::spawn_unit(UnitResource* ur, const Vector3& pos, const Quaternion& rot)
{
	// Allocate memory for unit
	Unit* unit = CE_NEW(m_unit_pool, Unit)(*this, ur, Matrix4x4(rot, pos));

	// Create Id for the unit
	const UnitId unit_id = m_units.create(unit);
	unit->set_id(unit_id);

	// SpawnUnitEvent ev;
	// ev.unit = unit_id;
	// event_stream::write(m_events, EventType::SPAWN, ev);

	return unit_id;
}

//-----------------------------------------------------------------------------
void World::destroy_unit(UnitId id)
{
	CE_ASSERT(m_units.has(id), "Unit does not exist");

	CE_DELETE(m_unit_pool, m_units.lookup(id));
	m_units.destroy(id);

	// DestroyUnitEvent ev;
	// ev.unit = id;
	// event_stream::write(m_events, EventType::DESTROY, ev);
}

//-----------------------------------------------------------------------------
void World::reload_units(UnitResource* old_ur, UnitResource* new_ur)
{
	for (uint32_t i = 0; i < m_units.size(); i++)
	{
		if (m_units[i]->resource() == old_ur)
		{
			m_units[i]->reload(new_ur);
		}
	}
}

//-----------------------------------------------------------------------------
uint32_t World::num_units() const
{
	return m_units.size();
}

//-----------------------------------------------------------------------------
void World::link_unit(UnitId child, UnitId parent, int32_t node)
{
	CE_ASSERT(m_units.has(child), "Child unit does not exist");
	CE_ASSERT(m_units.has(parent), "Parent unit does not exist");

	Unit* parent_unit = lookup_unit(parent);
	parent_unit->link_node(0, node);
}

//-----------------------------------------------------------------------------
void World::unlink_unit(UnitId child)
{
	CE_ASSERT(m_units.has(child), "Child unit does not exist");
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
	CE_ASSERT(m_cameras.has(camera), "Camera does not exist");

	return m_cameras.lookup(camera);
}

//-----------------------------------------------------------------------------
void World::update(float dt)
{
	// Update scene graphs
	m_scenegraph_manager.update();

	// Update physics world
	m_physics_world.update(dt);

	m_scenegraph_manager.update();

	m_sound_world->update();
}

//-----------------------------------------------------------------------------
void World::render(Camera* camera)
{
	m_render_world.update(camera->world_pose(), camera->m_projection, camera->m_view_x, camera->m_view_y,
							camera->m_view_width, camera->m_view_height, device()->last_delta_time());
}

//-----------------------------------------------------------------------------
CameraId World::create_camera(SceneGraph& sg, int32_t node)
{
	// Allocate memory for camera
	Camera* camera = CE_NEW(m_camera_pool, Camera)(sg, node);

	// Create Id for the camera
	const CameraId camera_id = m_cameras.create(camera);

	return camera_id;
}

//-----------------------------------------------------------------------------
void World::destroy_camera(CameraId id)
{
	CE_ASSERT(m_cameras.has(id), "Camera does not exist");

	CE_DELETE(m_camera_pool, m_cameras.lookup(id));
	m_cameras.destroy(id);
}

//-----------------------------------------------------------------------------
SoundInstanceId World::play_sound(const char* name, const bool loop, const float volume, const Vector3& pos, const float range)
{
	m_sound_world->play(name, loop, volume, pos);
}

//-----------------------------------------------------------------------------
void World::stop_sound(SoundInstanceId id)
{
	m_sound_world->stop(id);
}

//-----------------------------------------------------------------------------
void World::link_sound(SoundInstanceId id, Unit* unit, int32_t node)
{
}

//-----------------------------------------------------------------------------
void World::set_listener_pose(const Matrix4x4& pose)
{
	m_sound_world->set_listener_pose(pose);
}

//-----------------------------------------------------------------------------
void World::set_sound_position(SoundInstanceId id, const Vector3& pos)
{
	m_sound_world->set_sound_positions(1, &id, &pos);
}

//-----------------------------------------------------------------------------
void World::set_sound_range(SoundInstanceId id, float range)
{
	m_sound_world->set_sound_ranges(1, &id, &range);
}

//-----------------------------------------------------------------------------
void World::set_sound_volume(SoundInstanceId id, float vol)
{
	m_sound_world->set_sound_volumes(1, &id, &vol);
}

//-----------------------------------------------------------------------------
GuiId World::create_window_gui(const char* name)
{
	GuiResource* gr = (GuiResource*)device()->resource_manager()->lookup(GUI_EXTENSION, name);
	return m_render_world.create_gui(gr);
}

//-----------------------------------------------------------------------------
GuiId World::create_world_gui(const Matrix4x4 pose, const uint32_t width, const uint32_t height)
{
	// Must be implemented
}

//-----------------------------------------------------------------------------
void World::destroy_gui(GuiId id)
{
	m_render_world.destroy_gui(id);
}

//-----------------------------------------------------------------------------
Gui* World::lookup_gui(GuiId id)
{
	return m_render_world.lookup_gui(id);
}

//-----------------------------------------------------------------------------
SceneGraphManager* World::scene_graph_manager()
{
	return &m_scenegraph_manager;
}
//-----------------------------------------------------------------------------
RenderWorld* World::render_world()
{
	return &m_render_world;
}

//-----------------------------------------------------------------------------
PhysicsWorld* World::physics_world()
{
	return &m_physics_world;
}

//-----------------------------------------------------------------------------
SoundWorld* World::sound_world()
{
	return m_sound_world;
}

} // namespace crown
