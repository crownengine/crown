/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include <new>
#include "assert.h"
#include "world.h"
#include "device.h"
#include "resource_manager.h"
#include "debug_line.h"
#include "actor.h"
#include "lua_environment.h"
#include "level_resource.h"
#include "memory.h"
#include "matrix4x4.h"

namespace crown
{

World::World()
	: m_unit_pool(default_allocator(), CE_MAX_UNITS, sizeof(Unit), CE_ALIGNOF(Unit))
	, m_camera_pool(default_allocator(), CE_MAX_CAMERAS, sizeof(Camera), CE_ALIGNOF(Camera))
	, m_physics_world(*this)
	, m_events(default_allocator())
{
	m_id.id = INVALID_ID;
	m_sound_world = SoundWorld::create(default_allocator());
}

World::~World()
{
	// Destroy all units
	for (uint32_t i = 0; i < id_array::size(m_units); i++)
	{
		CE_DELETE(m_unit_pool, m_units[i]);
	}

	SoundWorld::destroy(default_allocator(), m_sound_world);
}

WorldId World::id() const
{
	return m_id;
}

void World::set_id(WorldId id)
{
	m_id = id;
}

UnitId World::spawn_unit(const char* name, const Vector3& pos, const Quaternion& rot)
{
	const ResourceId id(UNIT_EXTENSION, name);
	return spawn_unit(id.name, pos, rot);
}

UnitId World::spawn_unit(StringId64 name, const Vector3& pos, const Quaternion& rot)
{
	UnitResource* ur = (UnitResource*)device()->resource_manager()->get(UNIT_TYPE, name);

	Unit* u = (Unit*) m_unit_pool.allocate(sizeof(Unit), CE_ALIGNOF(Unit));
	const UnitId unit_id = id_array::create(m_units, u);
	new (u) Unit(*this, unit_id, name, ur, Matrix4x4(rot, pos));

	post_unit_spawned_event(unit_id);
	return unit_id;
}

void World::destroy_unit(UnitId id)
{
	CE_DELETE(m_unit_pool, id_array::get(m_units, id));
	id_array::destroy(m_units, id);
	post_unit_destroyed_event(id);
}

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

uint32_t World::num_units() const
{
	return id_array::size(m_units);
}

void World::units(Array<UnitId>& units) const
{
	for (uint32_t i = 0; i < id_array::size(m_units); i++)
	{
		array::push_back(units, m_units[i]->id());
	}
}

void World::link_unit(UnitId child, UnitId parent, int32_t node)
{
	Unit* parent_unit = get_unit(parent);
	parent_unit->link_node(0, node);
}

void World::unlink_unit(UnitId /*child*/)
{
}

Unit* World::get_unit(UnitId id)
{
	return id_array::get(m_units, id);
}

Camera* World::get_camera(CameraId id)
{
	return id_array::get(m_cameras, id);
}

void World::update_animations(float dt)
{
	m_sprite_animation_player.update(dt);
}

void World::update_scene(float dt)
{
	m_physics_world.update(dt);
	m_scenegraph_manager.update();

	for (uint32_t i = 0; i < id_array::size(m_units); i++)
	{
		m_units[i]->update();
	}

	m_sound_world->update();

	process_physics_events();
}

void World::update(float dt)
{
	update_animations(dt);
	update_scene(dt);
}

void World::render(Camera* camera)
{
	m_render_world.update(camera->view_matrix(), camera->projection_matrix(), camera->_view_x, camera->_view_y,
							camera->_view_width, camera->_view_height, device()->last_delta_time());

	m_physics_world.draw_debug();
}

CameraId World::create_camera(SceneGraph& sg, int32_t node, ProjectionType::Enum type, float near, float far)
{
	Camera* camera = CE_NEW(m_camera_pool, Camera)(sg, node, type, near, far);

	return id_array::create(m_cameras, camera);
}

void World::destroy_camera(CameraId id)
{
	CE_DELETE(m_camera_pool, id_array::get(m_cameras, id));
	id_array::destroy(m_cameras, id);
}

SoundInstanceId World::play_sound(const char* name, const bool loop, const float volume, const Vector3& pos, const float range)
{
	ResourceId id(SOUND_EXTENSION, name);
	return play_sound(id.name, loop, volume, pos, range);
}

SoundInstanceId World::play_sound(StringId64 name, const bool loop, const float volume, const Vector3& pos, const float range)
{
	SoundResource* sr = (SoundResource*)device()->resource_manager()->get(SOUND_TYPE, name);
	return m_sound_world->play(sr, loop, volume, pos);
}

void World::stop_sound(SoundInstanceId id)
{
	m_sound_world->stop(id);
}

void World::link_sound(SoundInstanceId id, Unit* unit, int32_t node)
{
}

void World::set_listener_pose(const Matrix4x4& pose)
{
	m_sound_world->set_listener_pose(pose);
}

void World::set_sound_position(SoundInstanceId id, const Vector3& pos)
{
	m_sound_world->set_sound_positions(1, &id, &pos);
}

void World::set_sound_range(SoundInstanceId id, float range)
{
	m_sound_world->set_sound_ranges(1, &id, &range);
}

void World::set_sound_volume(SoundInstanceId id, float vol)
{
	m_sound_world->set_sound_volumes(1, &id, &vol);
}

GuiId World::create_window_gui(uint16_t width, uint16_t height, const char* material)
{
	return m_render_world.create_gui(width, height, material);
}

void World::destroy_gui(GuiId id)
{
	m_render_world.destroy_gui(id);
}

Gui* World::get_gui(GuiId id)
{
	return m_render_world.get_gui(id);
}

DebugLine* World::create_debug_line(bool depth_test)
{
	return CE_NEW(default_allocator(), DebugLine)(depth_test);
}

void World::destroy_debug_line(DebugLine* line)
{
	CE_DELETE(default_allocator(), line);
}

void World::load_level(const char* name)
{
	const LevelResource* lr = (LevelResource*) device()->resource_manager()->get(LEVEL_EXTENSION, name);
	load_level(lr);
}

void World::load_level(const LevelResource* lr)
{
	using namespace level_resource;

	uint32_t num = level_resource::num_units(lr);
	for (uint32_t i = 0; i < num; i++)
	{
		const LevelUnit* lu = level_resource::get_unit(lr, i);
		spawn_unit(lu->name, lu->position, lu->rotation);
	}

	num = level_resource::num_sounds(lr);
	for (uint32_t i = 0; i < num; i++)
	{
		const LevelSound* ls = level_resource::get_sound(lr, i);
		play_sound(ls->name, ls->loop, ls->volume, ls->position, ls->range);
	}

	post_level_loaded_event();
}

SceneGraphManager* World::scene_graph_manager()
{
	return &m_scenegraph_manager;
}

SpriteAnimationPlayer* World::sprite_animation_player()
{
	return &m_sprite_animation_player;
}

RenderWorld* World::render_world()
{
	return &m_render_world;
}

PhysicsWorld* World::physics_world()
{
	return &m_physics_world;
}

SoundWorld* World::sound_world()
{
	return m_sound_world;
}

void World::post_unit_spawned_event(UnitId id)
{
	UnitSpawnedEvent ev;
	ev.unit = id;
	event_stream::write(m_events, EventType::UNIT_SPAWNED, ev);
}

void World::post_unit_destroyed_event(UnitId id)
{
	UnitDestroyedEvent ev;
	ev.unit = id;
	event_stream::write(m_events, EventType::UNIT_DESTROYED, ev);
}

void World::post_level_loaded_event()
{
	LevelLoadedEvent ev;
	event_stream::write(m_events, EventType::LEVEL_LOADED, ev);
}

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
