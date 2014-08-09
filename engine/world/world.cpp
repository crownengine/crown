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

#include <new>
#include "assert.h"
#include "world.h"
#include "allocator.h"
#include "device.h"
#include "resource_manager.h"
#include "debug_line.h"
#include "actor.h"
#include "lua_environment.h"
#include "level_resource.h"

namespace crown
{

//-----------------------------------------------------------------------------
World::World()
	: m_unit_pool(default_allocator(), CE_MAX_UNITS, sizeof(Unit), CE_ALIGNOF(Unit))
	, m_camera_pool(default_allocator(), CE_MAX_CAMERAS, sizeof(Camera), CE_ALIGNOF(Camera))
	, m_physics_world(*this)
	, m_events(default_allocator())
{
	m_id.id = INVALID_ID;
	m_sound_world = SoundWorld::create(default_allocator());
}

//-----------------------------------------------------------------------------
World::~World()
{
	// Destroy all units
	for (uint32_t i = 0; i < id_array::size(m_units); i++)
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
	const ResourceId id(UNIT_EXTENSION, name);
	return spawn_unit(id, pos, rot);
}

//-----------------------------------------------------------------------------
UnitId World::spawn_unit(ResourceId id, const Vector3& pos, const Quaternion& rot)
{
	UnitResource* ur = (UnitResource*) device()->resource_manager()->get(id);
	return spawn_unit(id, ur, pos, rot);
}

//-----------------------------------------------------------------------------
UnitId World::spawn_unit(const ResourceId id, UnitResource* ur, const Vector3& pos, const Quaternion& rot)
{
	Unit* u = (Unit*) m_unit_pool.allocate(sizeof(Unit), CE_ALIGNOF(Unit));
	const UnitId unit_id = id_array::create(m_units, u);
	new (u) Unit(*this, unit_id, id, ur, Matrix4x4(rot, pos));

	// SpawnUnitEvent ev;
	// ev.unit = unit_id;
	// event_stream::write(m_events, EventType::SPAWN, ev);

	return unit_id;
}

//-----------------------------------------------------------------------------
void World::destroy_unit(UnitId id)
{
	CE_DELETE(m_unit_pool, id_array::get(m_units, id));
	id_array::destroy(m_units, id);

	// DestroyUnitEvent ev;
	// ev.unit = id;
	// event_stream::write(m_events, EventType::DESTROY, ev);
}

//-----------------------------------------------------------------------------
void World::reload_units(UnitResource* old_ur, UnitResource* new_ur)
{
	for (uint32_t i = 0; i < id_array::size(m_units); i++)
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
	return id_array::size(m_units);
}

//-----------------------------------------------------------------------------
void World::units(Array<UnitId>& units) const
{
	for (uint32_t i = 0; i < id_array::size(m_units); i++)
	{
		array::push_back(units, m_units[i]->id());
	}
}

//-----------------------------------------------------------------------------
void World::link_unit(UnitId child, UnitId parent, int32_t node)
{
	Unit* parent_unit = get_unit(parent);
	parent_unit->link_node(0, node);
}

//-----------------------------------------------------------------------------
void World::unlink_unit(UnitId /*child*/)
{
}

//-----------------------------------------------------------------------------
Unit* World::get_unit(UnitId id)
{
	return id_array::get(m_units, id);
}

//-----------------------------------------------------------------------------
Camera* World::get_camera(CameraId id)
{
	return id_array::get(m_cameras, id);
}

//-----------------------------------------------------------------------------
void World::update(float dt)
{
	m_physics_world.update(dt);

	m_scenegraph_manager.update();

	m_sound_world->update();

	process_physics_events();
}

//-----------------------------------------------------------------------------
void World::render(Camera* camera)
{
	m_render_world.update(camera->world_pose(), camera->m_projection, camera->m_view_x, camera->m_view_y,
							camera->m_view_width, camera->m_view_height, device()->last_delta_time());

	m_physics_world.draw_debug();
}

//-----------------------------------------------------------------------------
CameraId World::create_camera(SceneGraph& sg, int32_t node, ProjectionType::Enum type, float near, float far)
{
	Camera* camera = CE_NEW(m_camera_pool, Camera)(sg, node, type, near, far);

	return id_array::create(m_cameras, camera);
}

//-----------------------------------------------------------------------------
void World::destroy_camera(CameraId id)
{
	CE_DELETE(m_camera_pool, id_array::get(m_cameras, id));
	id_array::destroy(m_cameras, id);
}

//-----------------------------------------------------------------------------
SoundInstanceId World::play_sound(const char* name, const bool loop, const float volume, const Vector3& pos, const float range)
{
	ResourceId id(SOUND_EXTENSION, name);
	return play_sound(id, loop, volume, pos, range);
}

//-----------------------------------------------------------------------------
SoundInstanceId World::play_sound(ResourceId id, const bool loop, const float volume, const Vector3& pos, const float range)
{
	SoundResource* sr = (SoundResource*) device()->resource_manager()->get(id);
	return play_sound(sr, loop, volume, pos, range);
}

//-----------------------------------------------------------------------------
SoundInstanceId World::play_sound(SoundResource* sr, const bool loop, const float volume, const Vector3& pos, const float range)
{
	return m_sound_world->play(sr, loop, volume, pos);
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
GuiId World::create_window_gui(uint16_t width, uint16_t height)
{
	return m_render_world.create_gui(width, height);
}

//-----------------------------------------------------------------------------
void World::destroy_gui(GuiId id)
{
	m_render_world.destroy_gui(id);
}

//-----------------------------------------------------------------------------
Gui* World::get_gui(GuiId id)
{
	return m_render_world.get_gui(id);
}

//-----------------------------------------------------------------------------
DebugLine* World::create_debug_line(bool depth_test)
{
	return CE_NEW(default_allocator(), DebugLine)(depth_test);
}

//-----------------------------------------------------------------------------
void World::destroy_debug_line(DebugLine* line)
{
	CE_DELETE(default_allocator(), line);
}

//-----------------------------------------------------------------------------
void World::load_level(const char* name)
{
	const LevelResource* res = (LevelResource*) device()->resource_manager()->get(LEVEL_EXTENSION, name);

	for (uint32_t i = 0; i < res->num_units(); i++)
	{
		const LevelUnit* lu = res->get_unit(i);
		spawn_unit(lu->name, lu->position, lu->rotation);
	}

	for (uint32_t i = 0; i < res->num_sounds(); i++)
	{
		const LevelSound* ls = res->get_sound(i);
		play_sound(ls->name, ls->loop, ls->volume, ls->position, ls->range);
	}
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

//-----------------------------------------------------------------------------
void World::process_physics_events()
{
	EventStream& events = m_physics_world.events();

	// Read all events
	const char* ee = array::begin(events);
	while (ee != array::end(events))
	{
		event_stream::Header h = *(event_stream::Header*) ee;

		// CE_LOGD("=== PHYSICS EVENT ===");
		// CE_LOGD("type = %d", h.type);
		// CE_LOGD("size = %d", h.size);

		const char* event = ee + sizeof(event_stream::Header);

		switch (h.type)
		{
			case physics_world::EventType::COLLISION:
			{
				physics_world::CollisionEvent coll_ev = *(physics_world::CollisionEvent*) event;

				// CE_LOGD("type    = %s", coll_ev.type == physics_world::CollisionEvent::BEGIN_TOUCH ? "begin" : "end");
				// CE_LOGD("actor_0 = (%p)", coll_ev.actors[0]);
				// CE_LOGD("actor_1 = (%p)", coll_ev.actors[1]);
				// CE_LOGD("unit_0  = (%p)", coll_ev.actors[0]->unit());
				// CE_LOGD("unit_1  = (%p)", coll_ev.actors[1]->unit());
				// CE_LOGD("where   = (%f %f %f)", coll_ev.where.x, coll_ev.where.y, coll_ev.where.z);
				// CE_LOGD("normal  = (%f %f %f)", coll_ev.normal.x, coll_ev.normal.y, coll_ev.normal.z);

				device()->lua_environment()->call_physics_callback(
					coll_ev.actors[0],
					coll_ev.actors[1],
					(id_array::has(m_units, coll_ev.actors[0]->unit_id())) ? coll_ev.actors[0]->unit() : NULL,
					(id_array::has(m_units, coll_ev.actors[1]->unit_id())) ? coll_ev.actors[1]->unit() : NULL,
					coll_ev.where,
					coll_ev.normal,
					(coll_ev.type == physics_world::CollisionEvent::BEGIN_TOUCH) ? "begin" : "end");
				break;
			}
			case physics_world::EventType::TRIGGER:
			{
				physics_world::TriggerEvent trigg_ev = *(physics_world::TriggerEvent*) event;

				// CE_LOGD("type    = %s", trigg_ev.type == physics_world::TriggerEvent::BEGIN_TOUCH ? "begin" : "end");
				// CE_LOGD("trigger = (%p)", trigg_ev.trigger);
				// CE_LOGD("other   = (%p)", trigg_ev.other);

				device()->lua_environment()->call_trigger_callback(
					trigg_ev.trigger,
					trigg_ev.other,
					(trigg_ev.type == physics_world::TriggerEvent::BEGIN_TOUCH ? "begin" : "end"));
				break;
			}
			default:
			{
				CE_FATAL("Unknown Physics event");
				break;
			}
		}

		// CE_LOGD("=====================");

		// Next event
		ee += sizeof(event_stream::Header) + h.size;
	}

	array::clear(events);
}

} // namespace crown
