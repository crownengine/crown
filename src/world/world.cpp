/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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
#include "device/log.h"
#include "lua/lua_environment.h"
#include "resource/level_resource.h"
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

LOG_SYSTEM(WORLD, "world")

namespace crown
{
static void create_components(World &w
	, const UnitResource *ur
	, const UnitId *unit_lookup
	, u32 spawn_flags
	, const Vector3 &pos
	, const Quaternion &rot
	, const Vector3 &scl
	)
{
	SceneGraph *scene_graph = w._scene_graph;
	RenderWorld *render_world = w._render_world;
	PhysicsWorld *physics_world = w._physics_world;
	ScriptWorld *script_world = w._script_world;
	AnimationStateMachine *animation_state_machine = w._animation_state_machine;

	const u32 *unit_parents = unit_resource::parents(ur);
	const ComponentData *component = unit_resource::component_type_data(ur, NULL);
	for (u32 cc = 0; cc < ur->num_component_types; ++cc) {
		const u32 *unit_index = unit_resource::component_unit_index(component);
		const char *data = unit_resource::component_payload(component);

		if (component->type == STRING_ID_32("transform", UINT32_C(0xad9b5315))) {
			scene_graph->create_instances(data
				, component->num_instances
				, unit_lookup
				, unit_index
				, unit_parents
				, spawn_flags
				, pos
				, rot
				, scl
				);
		} else if (component->type == STRING_ID_32("camera", UINT32_C(0x31822dc7))) {
			w.camera_create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("collider", UINT32_C(0x2129d74e))) {
			physics_world->collider_create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("actor", UINT32_C(0x374cf583))) {
			physics_world->actor_create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("mover", UINT32_C(0xac07d371))) {
			physics_world->mover_create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("mesh_renderer", UINT32_C(0xdf017893))) {
			render_world->_mesh_manager.create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("sprite_renderer", UINT32_C(0x6a1c2a3b))) {
			render_world->_sprite_manager.create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("light", UINT32_C(0xbb9f08c2))) {
			render_world->_light_manager.create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("fog", UINT32_C(0xf007ef0d))) {
			render_world->fog_create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("global_lighting", UINT32_C(0x718af7fe))) {
			render_world->global_lighting_create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("bloom", UINT32_C(0x995dd31c))) {
			render_world->bloom_create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("tonemap", UINT32_C(0x7089b06b))) {
			render_world->tonemap_create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("script", UINT32_C(0xd18f8ad6))) {
			script_world::create_instances(*script_world, data, component->num_instances, unit_lookup, unit_index);
		} else if (component->type == STRING_ID_32("animation_state_machine", UINT32_C(0xe87992ac))) {
			animation_state_machine->create_instances(data, component->num_instances, unit_lookup, unit_index);
		} else {
			CE_FATAL("Unknown component type");
		}

		component = unit_resource::component_type_data(ur, component);
	}
}

static void collect_units(Array<UnitId> *unit_lookup, SceneGraph *scene_graph, UnitId root)
{
	array::push_back(*unit_lookup, root);

	TransformInstance transform = scene_graph->instance(root);
	TransformInstance cur = scene_graph->first_child(transform);

	while (is_valid(cur)) {
		UnitId u = scene_graph->owner(cur);
		collect_units(unit_lookup, scene_graph, u);
		cur = scene_graph->next_sibling(cur);
	}
}

static void unit_destroyed_callback_bridge(UnitId unit, void *user_ptr)
{
	((World *)user_ptr)->unit_destroyed_callback(unit);
}

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
	, _skydome_unit(UNIT_INVALID)
#if CROWN_CAN_RELOAD
	, _unit_resources(a)
#endif
{
	_lines = create_debug_line(true);
	_scene_graph   = CE_NEW(*_allocator, SceneGraph)(*_allocator, um);
	_render_world  = CE_NEW(*_allocator, RenderWorld)(*_allocator, rm, sm, mm, um, pl, *_scene_graph);
	_physics_world = CE_NEW(*_allocator, PhysicsWorld)(*_allocator, rm, um, *_scene_graph, *_lines);
	_sound_world   = sound_world::create(*_allocator, rm);
	_script_world  = CE_NEW(*_allocator, ScriptWorld)(*_allocator, um, rm, env, *this);
	_sprite_animation_player = CE_NEW(*_allocator, SpriteAnimationPlayer)(*_allocator);
	_mesh_animation_player = CE_NEW(*_allocator, MeshAnimationPlayer)(*_allocator);
	_animation_state_machine = CE_NEW(*_allocator, AnimationStateMachine)(*_allocator, rm, um, *_sprite_animation_player, *_mesh_animation_player, *this);

	_gui_buffer.create();

	list::init_head(_guis);
	list::init_head(_levels);

	_node.next = NULL;
	_node.prev = NULL;

	_unit_destroy_callback.destroy = unit_destroyed_callback_bridge;
	_unit_destroy_callback.user_data = this;
	_unit_destroy_callback.node.next = NULL;
	_unit_destroy_callback.node.prev = NULL;
	um.register_destroy_callback(&_unit_destroy_callback);
}

World::~World()
{
	_unit_manager->unregister_destroy_callback(&_unit_destroy_callback);

	// Destroy loaded levels.
	ListNode *cur;
	ListNode *tmp;
	list_for_each_safe(cur, tmp, &_levels)
	{
		Level *level = (Level *)container_of(cur, Level, _node);
		destroy_level(*level);
	}

	// Destroy GUIs.
	list_for_each_safe(cur, tmp, &_guis)
	{
		Gui *gui = (Gui *)container_of(cur, Gui, _node);
		destroy_gui(*gui);
	}

	// Destroy subsystems
	CE_DELETE(*_allocator, _animation_state_machine);
	CE_DELETE(*_allocator, _mesh_animation_player);
	CE_DELETE(*_allocator, _sprite_animation_player);
	CE_DELETE(*_allocator, _script_world);
	sound_world::destroy(*_allocator, *_sound_world);
	CE_DELETE(*_allocator, _physics_world);
	CE_DELETE(*_allocator, _render_world);
	CE_DELETE(*_allocator, _scene_graph);
	destroy_debug_line(*_lines);

	// Destroy units.
	for (u32 i = 0; i < array::size(_units); ++i) {
		CE_ENSURE(_unit_manager->alive(_units[i]));
		_unit_manager->destroy(_units[i]);
	}

	_marker = 0;
}

UnitId World::spawn_unit(const UnitResource *ur, u32 flags, const Vector3 &pos, const Quaternion &rot, const Vector3 &scl)
{
	UnitId *unit_lookup = (UnitId *)default_scratch_allocator().allocate(sizeof(*unit_lookup) * ur->num_units);
	for (u32 i = 0; i < ur->num_units; ++i)
		unit_lookup[i] = _unit_manager->create();

	create_components(*this, ur, unit_lookup, flags, pos, rot, scl);

	array::push(_units, unit_lookup, ur->num_units);
#if CROWN_CAN_RELOAD
	array::push_back(_unit_resources, ur);
	for (u32 i = 0; i < ur->num_units - 1; ++i)
		array::push_back(_unit_resources, (const UnitResource *)NULL);
#endif
	post_unit_spawned_events(unit_lookup, ur->num_units);
	script_world::spawned(*_script_world, unit_lookup, ur->num_units);

	UnitId root_unit = unit_lookup[0];
	default_scratch_allocator().deallocate(unit_lookup);
	return root_unit;
}

UnitId World::spawn_unit(StringId64 name, u32 flags, const Vector3 &pos, const Quaternion &rot, const Vector3 &scl)
{
	const UnitResource *ur = (UnitResource *)_resource_manager->get(RESOURCE_TYPE_UNIT, name);
	return spawn_unit(ur, flags, pos, rot, scl);
}

UnitId World::spawn_empty_unit()
{
	UnitId unit = _unit_manager->create();
	array::push_back(_units, unit);
#if CROWN_CAN_RELOAD
	array::push_back(_unit_resources, (const UnitResource *)NULL);
#endif
	post_unit_spawned_events(&unit, 1);
	script_world::spawned(*_script_world, &unit, 1);
	return unit;
}

UnitId World::spawn_skydome(StringId64 skydome_name)
{
	if (!_skydome_unit.is_valid())
		_skydome_unit = spawn_unit(skydome_name);

	return _skydome_unit;
}

void World::destroy_unit(UnitId unit)
{
	Array<UnitId> unit_lookup(default_scratch_allocator());

	collect_units(&unit_lookup, _scene_graph, unit);
	script_world::unspawned(*_script_world, array::begin(unit_lookup), array::size(unit_lookup));

	for (u32 i = 0; i < array::size(unit_lookup); ++i) {
		_unit_manager->destroy(unit_lookup[i]);
		post_unit_destroyed_event(unit_lookup[i]);
	}

	remove_dead_units();
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

	// Process physics transform events.
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

			default:
				break;
			}
		}
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

	// Process collision events.
	{
		EventStream &events = _physics_world->events();
		const u32 size = array::size(events);
		u32 read = 0;
		while (read < size) {
			const EventHeader *eh = (EventHeader *)&events[read];
			const char *data = (char *)&eh[1];

			read += sizeof(*eh) + eh->size;

			switch (eh->type) {
			case EventType::PHYSICS_COLLISION: {
				const PhysicsCollisionEvent &ev = *(PhysicsCollisionEvent *)data;
				const char *funcs[] = { "collision_begin", "collision_stay", "collision_end" };
				const u32 nargs[] = { 8, 8, 3 };

				for (u32 i = 0; i < 2; ++i) {
					ScriptInstance inst = script_world::instance(*_script_world, ev.units[i]);

					if (is_valid(inst)) {
						ArgType::Enum arg_types[8] =
						{
							ArgType::POINTER,
							ArgType::UNIT,
							ArgType::UNIT,
							ArgType::ID,
							ArgType::ID,
							ArgType::VECTOR3,
							ArgType::VECTOR3,
							ArgType::FLOAT
						};

						Arg args[8];
						args[0].pointer_value = this;
						args[1].unit_value = ev.units[i];
						args[2].unit_value = ev.units[1 - i];
						args[3].id_value = ev.actors[i].i;
						args[4].id_value = ev.actors[1 - i].i;
						args[5].vector3_value = ev.position;
						args[6].vector3_value = ev.normal;
						args[7].float_value = ev.distance;

						script_world::unicast(*_script_world, funcs[ev.type], inst, arg_types, args, nargs[ev.type]);
					}
				}

				break;
			}

			case EventType::PHYSICS_TRIGGER: {
				const PhysicsTriggerEvent &ev = *(PhysicsTriggerEvent *)data;
				const char *funcs[] = { "trigger_enter", "trigger_leave" };
				ScriptInstance inst = script_world::instance(*_script_world, ev.trigger_unit);

				if (is_valid(inst)) {
					ArgType::Enum arg_types[3] =
					{
						ArgType::POINTER,
						ArgType::UNIT,
						ArgType::UNIT,
					};

					Arg args[3];
					args[0].pointer_value = this;
					args[1].unit_value = ev.trigger_unit;
					args[2].unit_value = ev.other_unit;

					script_world::unicast(*_script_world, funcs[ev.type], inst, arg_types, args, countof(args));
				}
				break;
			}

			default:
				break;
			}
		}

		array::clear(events);
	}

	ArgType::Enum arg_types[2] = { ArgType::POINTER, ArgType::FLOAT };
	Arg args[2];
	args[0].pointer_value = this;
	args[1].float_value = dt;
	script_world::broadcast(*_script_world, "update", arg_types, args, countof(args));
}

void World::update(f32 dt)
{
	update_animations(dt);
	update_scene(dt);
}

void World::render(const Matrix4x4 &view, const Matrix4x4 &proj, const Matrix4x4 &persp)
{
	_render_world->render(view, proj, persp, _skydome_unit, *_lines);

	_physics_world->debug_draw();
	_render_world->debug_draw(*_lines);

	_lines->submit();
	_lines->reset();
}

void World::camera_create_instances(const void *data, u32 num, const UnitId *unit_lookup, const u32 *unit_index)
{
	const CameraDesc *cameras = (CameraDesc *)data;

	for (u32 i = 0; i < num; ++i) {
		UnitId unit = unit_lookup[unit_index[i]];
		CE_ASSERT(!hash_map::has(_camera_map, unit), "Unit already has a camera component");

		Camera c;
		c.unit            = unit;
		c.projection_type = (ProjectionType::Enum)cameras[i].type;
		c.fov             = cameras[i].fov;
		c.near_range      = cameras[i].near_range;
		c.far_range       = cameras[i].far_range;

		const u32 last = array::size(_camera);
		array::push_back(_camera, c);

		hash_map::set(_camera_map, unit, last);
	}
}

CameraInstance World::camera_create(UnitId unit, const CameraDesc &cd)
{
	u32 unit_lookup = 0;
	camera_create_instances(&cd, 1, &unit, &unit_lookup);
	return camera_instance(unit);
}

void World::camera_destroy(CameraInstance camera)
{
	const u32 last = array::size(_camera) - 1;
	const UnitId u = _camera[camera.i].unit;
	const UnitId last_u = _camera[last].unit;

	_camera[camera.i] = _camera[last];
	array::pop_back(_camera);

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

Matrix4x4 World::camera_projection_matrix(CameraInstance camera, f32 aspect_ratio, ProjectionType::Enum projection_type)
{
	Camera &cam = _camera[camera.i];

	ProjectionType::Enum proj_type = projection_type == ProjectionType::COUNT
		? cam.projection_type
		: projection_type
		;

	const bgfx::Caps *caps = bgfx::getCaps();
	f32 bx_proj[16];
	switch (proj_type) {
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

SoundInstanceId World::play_sound(StringId64 name, const bool loop, const f32 volume, const f32 range, u32 flags, const Vector3 &pos, StringId32 group)
{
	return _sound_world->play(name, loop, volume, range, flags, pos, group);
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

Level *World::load_level(StringId64 name, u32 flags, const Vector3 &pos, const Quaternion &rot)
{
	const LevelResource *lr = (LevelResource *)_resource_manager->get(RESOURCE_TYPE_LEVEL, name);
	const UnitResource *ur = level_resource::unit_resource(lr);

	Level *level = level::create(*_allocator, lr);

	// Create IDs for units.
	for (u32 i = 0; i < ur->num_units; ++i)
		level->_unit_lookup[i] = _unit_manager->create();

	create_components(*this, ur, level->_unit_lookup, flags, pos, rot, VECTOR3_ONE);

	array::push(_units, level->_unit_lookup, ur->num_units);
#if CROWN_CAN_RELOAD
	array::push_back(_unit_resources, ur);
	for (u32 i = 0; i < ur->num_units - 1; ++i)
		array::push_back(_unit_resources, (const UnitResource *)NULL);
#endif

	post_unit_spawned_events(level->_unit_lookup, ur->num_units);
	script_world::spawned(*_script_world, level->_unit_lookup, ur->num_units);

	spawn_skydome(lr->skydome_unit);

	for (u32 i = 0; i < lr->num_sounds; ++i) {
		const LevelSound *ls = level_resource::get_sound(lr, i);
		play_sound(ls->name, ls->loop, ls->volume, ls->range, PlaySoundFlags::ENABLE_ATTENUATION, ls->position, ls->group);
	}

	list::add(level->_node, _levels);

	post_level_loaded_event();
	return level;
}

void World::destroy_level(Level &level)
{
	script_world::unspawned(*_script_world, level._unit_lookup, level._resource->num_units);

	for (u32 i = 0; i < level._resource->num_units; ++i) {
		const UnitId unit = level._unit_lookup[i];

		post_unit_destroyed_event(unit);
		_unit_manager->destroy(unit);
	}

	remove_dead_units();

	list::remove(level._node);
	level::destroy(*_allocator, &level);
}

void World::remove_dead_units()
{
	u32 n = array::size(_units);

	// Remove dead units from the units list.
	for (u32 i = n - 1; i != UINT32_MAX; --i) {
		if (!_unit_manager->alive(_units[i])) {
			_units[i] = _units[array::size(_units) - 1];
			array::pop_back(_units);
#if CROWN_CAN_RELOAD
			_unit_resources[i] = _unit_resources[array::size(_unit_resources) - 1];
			array::pop_back(_unit_resources);
#endif
		}
	}
}

void World::post_unit_spawned_events(UnitId *units, u32 num_units)
{
	for (u32 i = 0; i < num_units; ++i) {
		UnitSpawnedEvent ev;
		ev.unit = units[i];
		event_stream::write(_events, EventType::UNIT_SPAWNED, ev);
	}
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

void World::reload_units(const UnitResource *old_unit, const UnitResource *new_unit)
{
#if CROWN_CAN_RELOAD
	for (u32 i = 0; i < array::size(_unit_resources); ++i) {
		if (_unit_resources[i] == old_unit) {
			Vector3 pos = VECTOR3_ZERO;
			Quaternion rot = QUATERNION_IDENTITY;
			Vector3 scl = VECTOR3_ONE;
			TransformInstance ti = _scene_graph->instance(_units[i]);

			if (is_valid(ti)) {
				pos = _scene_graph->local_position(ti);
				rot = _scene_graph->local_rotation(ti);
				scl = _scene_graph->local_scale(ti);
			}

			// Collect units created when old_unit was spawned
			// and destroy all their components. Unit IDs will be reused.
			Array<UnitId> unit_lookup(default_scratch_allocator());
			collect_units(&unit_lookup, _scene_graph, _units[i]);

			for (u32 j = 0; j < array::size(unit_lookup); ++j)
				_unit_manager->trigger_destroy_callbacks(unit_lookup[j]);

			// Create or destroy IDs if new_unit has different number of units.
			u32 n = array::size(unit_lookup);
			for (u32 i = new_unit->num_units; i > n; --i) {
				UnitId u = _unit_manager->create();
				array::push_back(unit_lookup, u);
				array::push_back(_units, u);
				array::push_back(_unit_resources, (const UnitResource *)NULL);
			}
			for (u32 i = new_unit->num_units; i < n; ++i) {
				_unit_manager->destroy(array::back(unit_lookup));
				array::pop_back(unit_lookup);
			}
			CE_ENSURE(array::size(unit_lookup) == new_unit->num_units);

			create_components(*this
				, new_unit
				, array::begin(unit_lookup)
				, SpawnFlags::OVERRIDE_POSITION
				| SpawnFlags::OVERRIDE_ROTATION
				| SpawnFlags::OVERRIDE_SCALE
				, pos
				, rot
				, scl
				);
			_unit_resources[i] = new_unit;
		}
	}

	remove_dead_units();
#else
	CE_UNUSED_2(old_unit, new_unit);
	CE_NOOP();
#endif // if CROWN_CAN_RELOAD
}

void World::unit_destroyed_callback(UnitId unit)
{
	CameraInstance inst = camera_instance(unit);
	if (is_valid(inst))
		camera_destroy(inst);
}

} // namespace crown
