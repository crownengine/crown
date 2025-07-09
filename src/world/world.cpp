/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/hash_map.inl"
#include "core/error/error.h"
#include "core/list.inl"
#include "core/math/matrix4x4.inl"
#include "core/math/vector3.inl"
#include "core/math/vector4.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/string_id.inl"
#include "device/device.h"
#include "lua/lua_environment.h"
#include "resource/resource_manager.h"
#include "resource/unit_resource.h"
#include "world/animation_state_machine.h"
#include "world/debug_line.h"
#include "world/event_stream.inl"
#include "world/gui.h"
#include "world/level.h"
#include "world/physics_world.h"
#include "world/render_world.h"
#include "world/scene_graph.h"
#include "world/script_world.h"
#include "world/sound_world.h"
#include "world/unit_manager.h"
#include "world/world.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace crown
{
World::World(Allocator &a
	, ResourceManager &rm
	, ShaderManager &sm
	, MaterialManager &mm
	, UnitManager &um
	, LuaEnvironment &env
	, Pipeline &pl
	)
	: _marker(WORLD_MARKER)
	, _allocator(&a)
	, _resource_manager(&rm)
	, _shader_manager(&sm)
	, _material_manager(&mm)
	, _lua_environment(&env)
	, _pipeline(&pl)
	, _unit_manager(&um)
	, _lines(NULL)
	, _scene_graph(NULL)
	, _render_world(NULL)
	, _physics_world(NULL)
	, _sound_world(NULL)
	, _animation_state_machine(NULL)
	, _units(a)
	, _camera(a)
	, _camera_map(a)
	, _events(a)
	, _changed_units(a)
	, _changed_world(a)
	, _gui_buffer(sm)
{
	_lines = create_debug_line(true);
	_scene_graph   = CE_NEW(*_allocator, SceneGraph)(*_allocator, um);
	_render_world  = CE_NEW(*_allocator, RenderWorld)(*_allocator, rm, sm, mm, um, pl, *_scene_graph);
	_physics_world = CE_NEW(*_allocator, PhysicsWorld)(*_allocator, rm, um, *_lines);
	_sound_world   = CE_NEW(*_allocator, SoundWorld)(*_allocator, rm);
	_script_world  = CE_NEW(*_allocator, ScriptWorld)(*_allocator, um, rm, env, *this);
	_sprite_animation_player = CE_NEW(*_allocator, SpriteAnimationPlayer)(*_allocator);
	_mesh_animation_player = CE_NEW(*_allocator, MeshAnimationPlayer)(*_allocator);
	_animation_state_machine = CE_NEW(*_allocator, AnimationStateMachine)(*_allocator, rm, um, *_sprite_animation_player, *_mesh_animation_player);

	_gui_buffer.create();

	list::init_head(_guis);
	list::init_head(_levels);

	_node.next = NULL;
	_node.prev = NULL;
}

World::~World()
{
	// Destroy loaded levels
	ListNode *cur;
	ListNode *tmp;
	list_for_each_safe(cur, tmp, &_levels)
	{
		Level *level = (Level *)container_of(cur, Level, _node);
		CE_DELETE(*_allocator, level);
	}

	// Destroy units
	for (u32 i = 0; i < array::size(_units); ++i)
		_unit_manager->destroy(_units[i]);

	// Destroy subsystems
	CE_DELETE(*_allocator, _animation_state_machine);
	CE_DELETE(*_allocator, _mesh_animation_player);
	CE_DELETE(*_allocator, _sprite_animation_player);
	CE_DELETE(*_allocator, _script_world);
	CE_DELETE(*_allocator, _sound_world);
	CE_DELETE(*_allocator, _physics_world);
	CE_DELETE(*_allocator, _render_world);
	CE_DELETE(*_allocator, _scene_graph);
	destroy_debug_line(*_lines);

	_marker = 0;
}

UnitId World::spawn_unit(StringId64 name, const Vector3 &pos, const Quaternion &rot, const Vector3 &scl)
{
	const UnitResource *ur = (const UnitResource *)_resource_manager->get(RESOURCE_TYPE_UNIT, name);

	UnitId *unit_lookup = (UnitId *)default_scratch_allocator().allocate(sizeof(*unit_lookup) * ur->num_units);
	for (u32 i = 0; i < ur->num_units; ++i)
		unit_lookup[i] = _unit_manager->create();

	spawn_units(*this, ur, pos, rot, scl, unit_lookup);

	UnitId root_unit = unit_lookup[0];
	default_scratch_allocator().deallocate(unit_lookup);
	return root_unit;
}

UnitId World::spawn_empty_unit()
{
	UnitId unit = _unit_manager->create();
	array::push_back(_units, unit);
	post_unit_spawned_event(unit);
	return unit;
}

void World::destroy_unit(UnitId unit)
{
	_unit_manager->destroy(unit);
	for (u32 i = 0, n = array::size(_units); i < n; ++i) {
		if (_units[i] == unit) {
			_units[i] = _units[n - 1];
			array::pop_back(_units);
			break;
		}
	}
	post_unit_destroyed_event(unit);
}

u32 World::num_units() const
{
	return array::size(_units);
}

void World::units(Array<UnitId> &units) const
{
	array::reserve(units, array::size(_units));
	array::push(units, array::begin(_units), array::size(_units));
}

UnitId World::unit_by_name(StringId32 name)
{
	ListNode *cur;
	list_for_each(cur, &_levels)
	{
		Level *level = (Level *)container_of(cur, Level, _node);
		UnitId unit = level->unit_by_name(name);

		if (unit != UNIT_INVALID)
			return unit;
	}

	return UNIT_INVALID;
}

void World::update_animations(f32 dt)
{
	_animation_state_machine->update(dt, *_scene_graph);
}

void World::update_scene(f32 dt)
{
	// Process animation events
	{
		EventStream &events = _animation_state_machine->_events;
		const u32 size = array::size(events);
		u32 read = 0;
		while (read < size) {
			const EventHeader *eh = (EventHeader *)&events[read];
			const char *data = (char *)&eh[1];

			read += sizeof(*eh) + eh->size;

			switch (eh->type) {
			case 0: {
				const SpriteFrameChangeEvent &ptev = *(SpriteFrameChangeEvent *)data;
				const SpriteInstance si = _render_world->sprite_instance(ptev.unit);
				_render_world->sprite_set_frame(si, ptev.frame_num);
				break;
			}

			default:
				CE_FATAL("Unknown event type");
				break;
			}
		}
		array::clear(events);
	}

	_scene_graph->get_changed(_changed_units, _changed_world);

	_physics_world->update_actor_world_poses(array::begin(_changed_units)
		, array::end(_changed_units)
		, array::begin(_changed_world)
		);

	_physics_world->update(dt);

	// Process physics events
	{
		EventStream &events = _physics_world->events();
		const u32 size = array::size(events);
		u32 read = 0;
		while (read < size) {
			const EventHeader *eh = (EventHeader *)&events[read];
			const char *data = (char *)&eh[1];

			read += sizeof(*eh) + eh->size;

			switch (eh->type) {
			case EventType::PHYSICS_TRANSFORM: {
				const PhysicsTransformEvent &ptev = *(PhysicsTransformEvent *)data;
				const TransformInstance ti = _scene_graph->instance(ptev.unit_id);
				if (is_valid(ti)) // User code may have destroyed the actor
					_scene_graph->set_world_pose_and_rescale(ti, ptev.world);
				break;
			}

			case EventType::PHYSICS_COLLISION: {
				const PhysicsCollisionEvent &pcev = *(PhysicsCollisionEvent *)data;
				script_world::collision(*_script_world, pcev);
				break;
			}

			case EventType::PHYSICS_TRIGGER:
				break;

			default:
				CE_FATAL("Unknown event type");
				break;
			}
		}
		array::clear(events);
	}

	array::clear(_changed_units);
	array::clear(_changed_world);
	_scene_graph->get_changed(_changed_units, _changed_world);
	_scene_graph->clear_changed();

	_render_world->update_transforms(array::begin(_changed_units)
		, array::end(_changed_units)
		, array::begin(_changed_world)
		);

	array::clear(_changed_units);
	array::clear(_changed_world);

	_sound_world->update();

	_gui_buffer.reset();

	array::clear(_events);

	script_world::update(*_script_world, dt);
}

void World::update(f32 dt)
{
	update_animations(dt);
	update_scene(dt);
}

void World::render(const Matrix4x4 &view, const Matrix4x4 &proj)
{
	_render_world->render(view, proj);

	_physics_world->debug_draw();
	_render_world->debug_draw(*_lines);

	_lines->submit();
	_lines->reset();
}

CameraInstance World::camera_create(UnitId unit, const CameraDesc &cd, const Matrix4x4 & /*tr*/)
{
	CE_ASSERT(!hash_map::has(_camera_map, unit), "Unit already has a camera component");

	Camera camera;
	camera.unit            = unit;
	camera.projection_type = (ProjectionType::Enum)cd.type;
	camera.fov             = cd.fov;
	camera.near_range      = cd.near_range;
	camera.far_range       = cd.far_range;

	const u32 last = array::size(_camera);
	array::push_back(_camera, camera);

	hash_map::set(_camera_map, unit, last);
	return camera_make_instance(last);
}

void World::camera_destroy(CameraInstance camera)
{
	const u32 last = array::size(_camera) - 1;
	const UnitId u = _camera[camera.i].unit;
	const UnitId last_u = _camera[last].unit;

	_camera[camera.i] = _camera[last];

	hash_map::set(_camera_map, last_u, camera.i);
	hash_map::remove(_camera_map, u);
}

CameraInstance World::camera_instance(UnitId unit)
{
	return camera_make_instance(hash_map::get(_camera_map, unit, UINT32_MAX));
}

void World::camera_set_projection_type(CameraInstance camera, ProjectionType::Enum type)
{
	_camera[camera.i].projection_type = type;
}

ProjectionType::Enum World::camera_projection_type(CameraInstance camera)
{
	return _camera[camera.i].projection_type;
}

Matrix4x4 World::camera_projection_matrix(CameraInstance camera, f32 aspect_ratio)
{
	Camera &cam = _camera[camera.i];

	const bgfx::Caps *caps = bgfx::getCaps();
	f32 bx_proj[16];
	switch (cam.projection_type) {
	case ProjectionType::ORTHOGRAPHIC:
		bx::mtxOrtho(bx_proj
			, -cam.half_size * aspect_ratio
			, cam.half_size * aspect_ratio
			, -cam.half_size
			, cam.half_size
			, cam.near_range
			, cam.far_range
			, 0.0f
			, caps->homogeneousDepth
			, bx::Handedness::Right
			);
		break;

	case ProjectionType::PERSPECTIVE:
		bx::mtxProj(bx_proj
			, fdeg(cam.fov)
			, aspect_ratio
			, cam.near_range
			, cam.far_range
			, caps->homogeneousDepth
			, bx::Handedness::Right
			);
		break;

	default:
		CE_FATAL("Unknown projection type");
		break;
	}

	return from_array(bx_proj);
}

Matrix4x4 World::camera_view_matrix(CameraInstance camera)
{
	TransformInstance ti = _scene_graph->instance(_camera[camera.i].unit);
	Matrix4x4 view = _scene_graph->world_pose(ti);
	Matrix4x4 rotate_x_90 = from_matrix3x3(from_x_axis_angle(frad(90.0f)));

	view = rotate_x_90 * view;
	invert(view);
	return view;
}

f32 World::camera_fov(CameraInstance camera)
{
	return _camera[camera.i].fov;
}

void World::camera_set_fov(CameraInstance camera, f32 fov)
{
	_camera[camera.i].fov = fov;
}

f32 World::camera_near_clip_distance(CameraInstance camera)
{
	return _camera[camera.i].near_range;
}

void World::camera_set_near_clip_distance(CameraInstance camera, f32 near)
{
	_camera[camera.i].near_range = near;
}

f32 World::camera_far_clip_distance(CameraInstance camera)
{
	return _camera[camera.i].far_range;
}

void World::camera_set_far_clip_distance(CameraInstance camera, f32 far)
{
	_camera[camera.i].far_range = far;
}

void World::camera_set_orthographic_size(CameraInstance camera, f32 half_size)
{
	_camera[camera.i].half_size = half_size;
}

Vector3 World::camera_screen_to_world(CameraInstance camera, const Vector3 &pos)
{
	u16 view_width;
	u16 view_height;
	device()->resolution(view_width, view_height);
	const f32 w = (f32)view_width;
	const f32 h = (f32)view_height;

	Matrix4x4 camera_proj = camera_projection_matrix(camera, w/h);
	Matrix4x4 camera_view = camera_view_matrix(camera);
	Matrix4x4 camera_view_proj = camera_view * camera_proj;
	invert(camera_view_proj);

	const bgfx::Caps *caps = bgfx::getCaps();

	Vector4 ndc;
	ndc.x = (2.0f * pos.x) / w - 1.0f;
	ndc.y = (2.0f * pos.y) / h - 1.0f;
	ndc.z = caps->homogeneousDepth ? (2.0f * pos.z) - 1.0f : pos.z;
	ndc.w = 1.0f;

	Vector4 tmp = ndc * camera_view_proj;
	tmp *= 1.0f / tmp.w;

	return { tmp.x, tmp.y, tmp.z };
}

Vector3 World::camera_world_to_screen(CameraInstance camera, const Vector3 &pos)
{
	u16 view_width;
	u16 view_height;
	device()->resolution(view_width, view_height);
	const f32 x = 0.0f;
	const f32 y = 0.0f;
	const f32 w = (f32)view_width;
	const f32 h = (f32)view_height;

	Matrix4x4 camera_proj = camera_projection_matrix(camera, w/h);
	Matrix4x4 camera_view = camera_view_matrix(camera);
	Matrix4x4 camera_view_proj = camera_view * camera_proj;

	Vector4 pos4 = { pos.x, pos.y, pos.z, 1.0f };
	Vector4 ndc  = pos4 * camera_view_proj;
	ndc.x *= 1.0 / ndc.w;
	ndc.y *= 1.0 / ndc.w;

	Vector3 screen;
	screen.x = (x + w  * (ndc.x + 1.0f)) / 2.0f;
	screen.y = h - (y + h * (1.0f - ndc.y)) / 2.0f;
	screen.z = ndc.z;

	return screen;
}

SoundInstanceId World::play_sound(StringId64 name, const bool loop, const f32 volume, const Vector3 &pos, const f32 range, StringId32 group)
{
	return _sound_world->play(name, loop, volume, range, pos, group);
}

void World::stop_sound(SoundInstanceId id)
{
	_sound_world->stop(id);
}

void World::link_sound(SoundInstanceId /*id*/, UnitId /*unit*/, s32 /*node*/)
{
	CE_FATAL("Not implemented yet");
}

void World::set_listener_pose(const Matrix4x4 &pose)
{
	_sound_world->set_listener_pose(pose);
}

void World::set_sound_position(SoundInstanceId id, const Vector3 &pos)
{
	_sound_world->set_sound_positions(1, &id, &pos);
}

void World::set_sound_range(SoundInstanceId id, f32 range)
{
	_sound_world->set_sound_ranges(1, &id, &range);
}

void World::set_sound_volume(SoundInstanceId id, f32 vol)
{
	_sound_world->set_sound_volumes(1, &id, &vol);
}

DebugLine *World::create_debug_line(bool depth_test)
{
	return debug_line::create(*_allocator, *_pipeline, depth_test);
}

void World::destroy_debug_line(DebugLine &line)
{
	CE_DELETE(*_allocator, &line);
}

Gui *World::create_screen_gui()
{
	Gui *gui = gui::create_screen_gui(*_allocator
		, _gui_buffer
		, *_resource_manager
		, *_shader_manager
		, *_material_manager
		, &_pipeline->_gui_shader
		);

	list::add(gui->_node, _guis);
	return gui;
}

Gui *World::create_world_gui()
{
	Gui *gui = gui::create_world_gui(*_allocator
		, _gui_buffer
		, *_resource_manager
		, *_shader_manager
		, *_material_manager
		, &_pipeline->_gui_3d_shader
		);

	list::add(gui->_node, _guis);
	return gui;
}

void World::destroy_gui(Gui &gui)
{
	list::remove(gui._node);
	CE_DELETE(*_allocator, &gui);
}

Level *World::load_level(StringId64 name, const Vector3 &pos, const Quaternion &rot)
{
	const LevelResource *lr = (const LevelResource *)_resource_manager->get(RESOURCE_TYPE_LEVEL, name);

	Level *level = CE_NEW(*_allocator, Level)(*_allocator, *_unit_manager, *this, *lr);
	level->load(pos, rot);

	list::add(level->_node, _levels);

	post_level_loaded_event();
	return level;
}

void World::post_unit_spawned_event(UnitId unit)
{
	UnitSpawnedEvent ev;
	ev.unit = unit;
	event_stream::write(_events, EventType::UNIT_SPAWNED, ev);
}

void World::post_unit_destroyed_event(UnitId unit)
{
	UnitDestroyedEvent ev;
	ev.unit = unit;
	event_stream::write(_events, EventType::UNIT_DESTROYED, ev);
}

void World::post_level_loaded_event()
{
	LevelLoadedEvent ev;
	event_stream::write(_events, EventType::LEVEL_LOADED, ev);
}

void World::disable_unit_callbacks()
{
#if CROWN_DEBUG
	_script_world->_disable_callbacks = true;
#endif
}

void World::reload_materials(const MaterialResource *old_resource, const MaterialResource *new_resource)
{
#if CROWN_CAN_RELOAD
	_render_world->reload_materials(old_resource, new_resource);
#else
	CE_UNUSED_2(old_resource, new_resource);
	CE_NOOP();
#endif
}

void spawn_units(World &w, const UnitResource *ur, const Vector3 &pos, const Quaternion &rot, const Vector3 &scl, const UnitId *unit_lookup)
{
	SceneGraph *scene_graph = w._scene_graph;
	RenderWorld *render_world = w._render_world;
	PhysicsWorld *physics_world = w._physics_world;
	ScriptWorld *script_world = w._script_world;
	AnimationStateMachine *animation_state_machine = w._animation_state_machine;

	const u32 *unit_parents = unit_resource::parents(ur);

	// Create components
	const ComponentData *component = unit_resource::component_type_data(ur, NULL);
	for (u32 cc = 0; cc < ur->num_component_types; ++cc) {
		const u32 *unit_index = unit_resource::component_unit_index(component);
		const char *data = unit_resource::component_payload(component);

		if (component->type == STRING_ID_32("transform", UINT32_C(0xad9b5315))) {
			const TransformDesc *td = (const TransformDesc *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i, ++td) {
				// FIXME: add SceneGraph::allocate() to reserve an instance
				// without initializing it.
				const TransformInstance ti = scene_graph->create(unit_lookup[unit_index[i]]
					, td->position
					, td->rotation
					, td->scale
					);
				if (unit_parents[unit_index[i]] != UINT32_MAX) {
					TransformInstance parent_ti = scene_graph->instance(unit_lookup[unit_parents[unit_index[i]]]);
					scene_graph->link(parent_ti, ti, td->position, td->rotation, td->scale);
				} else {
					const Vector3 scale = { td->scale.x * scl.x, td->scale.y * scl.y, td->scale.z * scl.z };
					Matrix4x4 tr = from_quaternion_translation(rot, pos);
					scene_graph->set_local_pose(ti, scene_graph->local_pose(ti) * tr);
					scene_graph->set_local_scale(ti, scale);
				}
			}
		} else if (component->type == STRING_ID_32("camera", UINT32_C(0x31822dc7))) {
			const CameraDesc *cd = (const CameraDesc *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i, ++cd) {
				w.camera_create(unit_lookup[unit_index[i]], *cd, MATRIX4X4_IDENTITY);
			}
		} else if (component->type == STRING_ID_32("collider", UINT32_C(0x2129d74e))) {
			const ColliderDesc *cd = (const ColliderDesc *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i) {
				TransformInstance ti = scene_graph->instance(unit_lookup[unit_index[i]]);
				Matrix4x4 tm = scene_graph->world_pose(ti);
				physics_world->collider_create(unit_lookup[unit_index[i]], cd, scale(tm));
				cd = (ColliderDesc *)((char *)(cd + 1) + cd->size);
			}
		} else if (component->type == STRING_ID_32("actor", UINT32_C(0x374cf583))) {
			const ActorResource *ar = (const ActorResource *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i, ++ar) {
				TransformInstance ti = scene_graph->instance(unit_lookup[unit_index[i]]);
				Matrix4x4 tm = scene_graph->world_pose(ti);
				physics_world->actor_create(unit_lookup[unit_index[i]], ar, from_quaternion_translation(rotation(tm), translation(tm)));
			}
		} else if (component->type == STRING_ID_32("mesh_renderer", UINT32_C(0xdf017893))) {
			const MeshRendererDesc *mrd = (const MeshRendererDesc *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i, ++mrd) {
				TransformInstance ti = scene_graph->instance(unit_lookup[unit_index[i]]);
				Matrix4x4 tm = scene_graph->world_pose(ti);
				render_world->mesh_create(unit_lookup[unit_index[i]], *mrd, tm);
			}
		} else if (component->type == STRING_ID_32("sprite_renderer", UINT32_C(0x6a1c2a3b))) {
			const SpriteRendererDesc *srd = (const SpriteRendererDesc *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i, ++srd) {
				TransformInstance ti = scene_graph->instance(unit_lookup[unit_index[i]]);
				Matrix4x4 tm = scene_graph->world_pose(ti);
				render_world->sprite_create(unit_lookup[unit_index[i]], *srd, tm);
			}
		} else if (component->type == STRING_ID_32("light", UINT32_C(0xbb9f08c2))) {
			const LightDesc *ld = (const LightDesc *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i, ++ld) {
				TransformInstance ti = scene_graph->instance(unit_lookup[unit_index[i]]);
				Matrix4x4 tm = scene_graph->world_pose(ti);
				render_world->light_create(unit_lookup[unit_index[i]], *ld, tm);
			}
		} else if (component->type == STRING_ID_32("script", UINT32_C(0xd18f8ad6))) {
			const ScriptDesc *sd = (const ScriptDesc *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i, ++sd) {
				script_world::create(*script_world, unit_lookup[unit_index[i]], *sd);
			}
		} else if (component->type == STRING_ID_32("animation_state_machine", UINT32_C(0xe87992ac))) {
			const AnimationStateMachineDesc *asmd = (const AnimationStateMachineDesc *)data;
			for (u32 i = 0, n = component->num_instances; i < n; ++i, ++asmd) {
				animation_state_machine->create(unit_lookup[unit_index[i]], *asmd, w);
			}
		} else {
			CE_FATAL("Unknown component type");
		}

		component = unit_resource::component_type_data(ur, component);
	}

	for (u32 i = 0; i < ur->num_units; ++i)
		array::push_back(w._units, unit_lookup[i]);

	// Post events
	for (u32 i = 0; i < ur->num_units; ++i)
		w.post_unit_spawned_event(unit_lookup[i]);
}

} // namespace crown
