/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "world.h"
#include "error.h"
#include "resource_manager.h"
#include "debug_line.h"
#include "actor.h"
#include "lua_environment.h"
#include "level_resource.h"
#include "memory.h"
#include "matrix4x4.h"
#include "int_setting.h"
#include <new>

namespace crown
{

static IntSetting g_physics_debug("physics.debug", "Enable physics debug rendering.", 0, 0, 1);

World::World(ResourceManager& rm, LuaEnvironment& env)
	: _resource_manager(&rm)
	, _lua_environment(&env)
	, m_unit_pool(default_allocator(), CE_MAX_UNITS, sizeof(Unit), CE_ALIGNOF(Unit))
	, m_camera_pool(default_allocator(), CE_MAX_CAMERAS, sizeof(Camera), CE_ALIGNOF(Camera))
	, _scenegraph_manager(NULL)
	, _sprite_animation_player(NULL)
	, _render_world(NULL)
	, _physics_world(NULL)
	, _sound_world(NULL)
	, _events(default_allocator())
	, _lines(NULL)
{
	_scenegraph_manager = CE_NEW(default_allocator(), SceneGraphManager);
	_sprite_animation_player = CE_NEW(default_allocator(), SpriteAnimationPlayer);
	_render_world = CE_NEW(default_allocator(), RenderWorld);
	_physics_world = CE_NEW(default_allocator(), PhysicsWorld)(*this);
	_sound_world = SoundWorld::create(default_allocator());
	_lines = create_debug_line(false);
}

World::~World()
{
	// Destroy all units
	for (uint32_t i = 0; i < id_array::size(m_units); i++)
	{
		CE_DELETE(m_unit_pool, m_units[i]);
	}

	destroy_debug_line(_lines);
	SoundWorld::destroy(default_allocator(), _sound_world);
	CE_DELETE(default_allocator(), _physics_world);
	CE_DELETE(default_allocator(), _render_world);
	CE_DELETE(default_allocator(), _sprite_animation_player);
	CE_DELETE(default_allocator(), _scenegraph_manager);
}

UnitId World::spawn_unit(const char* name, const Vector3& pos, const Quaternion& rot)
{
	const ResourceId id(UNIT_EXTENSION, name);
	return spawn_unit(id.name, pos, rot);
}

UnitId World::spawn_unit(StringId64 name, const Vector3& pos, const Quaternion& rot)
{
	UnitResource* ur = (UnitResource*)_resource_manager->get(UNIT_TYPE, name);

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
	_sprite_animation_player->update(dt);
}

void World::update_scene(float dt)
{
	_physics_world->update(dt);
	_scenegraph_manager->update();

	for (uint32_t i = 0; i < id_array::size(m_units); i++)
	{
		m_units[i]->update();
	}

	_sound_world->update();

	process_physics_events();
}

void World::update(float dt)
{
	update_animations(dt);
	update_scene(dt);
}

void World::render(Camera* camera)
{
	_render_world->update(camera->view_matrix(), camera->projection_matrix(), camera->_view_x, camera->_view_y,
		camera->_view_width, camera->_view_height);

	if (g_physics_debug == 1)
		_physics_world->draw_debug(*_lines);
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
	SoundResource* sr = (SoundResource*)_resource_manager->get(SOUND_TYPE, name);
	return _sound_world->play(sr, loop, volume, pos);
}

void World::stop_sound(SoundInstanceId id)
{
	_sound_world->stop(id);
}

void World::link_sound(SoundInstanceId id, Unit* unit, int32_t node)
{
}

void World::set_listener_pose(const Matrix4x4& pose)
{
	_sound_world->set_listener_pose(pose);
}

void World::set_sound_position(SoundInstanceId id, const Vector3& pos)
{
	_sound_world->set_sound_positions(1, &id, &pos);
}

void World::set_sound_range(SoundInstanceId id, float range)
{
	_sound_world->set_sound_ranges(1, &id, &range);
}

void World::set_sound_volume(SoundInstanceId id, float vol)
{
	_sound_world->set_sound_volumes(1, &id, &vol);
}

GuiId World::create_window_gui(uint16_t width, uint16_t height, const char* material)
{
	return _render_world->create_gui(width, height, material);
}

void World::destroy_gui(GuiId id)
{
	_render_world->destroy_gui(id);
}

Gui* World::get_gui(GuiId id)
{
	return _render_world->get_gui(id);
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
	const LevelResource* lr = (LevelResource*) _resource_manager->get(LEVEL_EXTENSION, name);
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
	return _scenegraph_manager;
}

SpriteAnimationPlayer* World::sprite_animation_player()
{
	return _sprite_animation_player;
}

RenderWorld* World::render_world()
{
	return _render_world;
}

PhysicsWorld* World::physics_world()
{
	return _physics_world;
}

SoundWorld* World::sound_world()
{
	return _sound_world;
}

void World::post_unit_spawned_event(UnitId id)
{
	UnitSpawnedEvent ev;
	ev.unit = id;
	event_stream::write(_events, EventType::UNIT_SPAWNED, ev);
}

void World::post_unit_destroyed_event(UnitId id)
{
	UnitDestroyedEvent ev;
	ev.unit = id;
	event_stream::write(_events, EventType::UNIT_DESTROYED, ev);
}

void World::post_level_loaded_event()
{
	LevelLoadedEvent ev;
	event_stream::write(_events, EventType::LEVEL_LOADED, ev);
}

void World::process_physics_events()
{
	EventStream& events = _physics_world->events();

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

				_lua_environment->call_physics_callback(
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

				_lua_environment->call_trigger_callback(
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
