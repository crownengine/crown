/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/types.h"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/vector3.inl"
#include "core/memory/globals.h"
#include "core/strings/string_id.inl"
#include "resource/expression_language.h"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"
#include "resource/state_machine_resource.h"
#include "world/animation_state_machine.h"
#include "world/event_stream.inl"
#include "world/render_world.h"
#include "world/types.h"
#include "world/unit_manager.h"
#include "world/world.h"
#include "device/log.h"

LOG_SYSTEM(ANIMATION_STATE_MACHINE, "animation_state_machine")

namespace crown
{
static void unit_destroyed_callback_bridge(UnitId unit, void *user_ptr)
{
	((AnimationStateMachine *)user_ptr)->unit_destroyed_callback(unit);
}

static StateMachineInstance make_instance(u32 i)
{
	StateMachineInstance inst = { i }; return inst;
}

AnimationStateMachine::AnimationStateMachine(Allocator &a
	, ResourceManager &rm
	, UnitManager &um
	, SpriteAnimationPlayer &sprite_player
	, MeshAnimationPlayer &mesh_player
	, World &world
	)
	: _marker(ANIMATION_STATE_MACHINE_MARKER)
	, _resource_manager(&rm)
	, _unit_manager(&um)
	, _map(a)
	, _machines(a)
	, _events(a)
	, _sprite_animation_player(&sprite_player)
	, _mesh_animation_player(&mesh_player)
	, _world(&world)
{
	_unit_destroy_callback.destroy = unit_destroyed_callback_bridge;
	_unit_destroy_callback.user_data = this;
	_unit_destroy_callback.node.next = NULL;
	_unit_destroy_callback.node.prev = NULL;
	um.register_destroy_callback(&_unit_destroy_callback);
}

AnimationStateMachine::~AnimationStateMachine()
{
	_unit_manager->unregister_destroy_callback(&_unit_destroy_callback);
	_marker = 0;

	for (u32 i = 0; i < array::size(_machines); ++i) {
		Machine &machine = _machines[i];

		deallocate(machine);
	}
}

static void mesh_set_skeleton_recursively(UnitId unit, AnimationSkeletonInstance *skeleton, SceneGraph &scene_graph, RenderWorld &render_world)
{
	// Set skeleton in this unit and all its children.
	UnitId child_id = unit;
	MeshInstance mesh = render_world.mesh_instance(child_id);
	if (is_valid(mesh)) {
		render_world.mesh_set_skeleton(mesh, skeleton);
	}

	TransformInstance transform = scene_graph.instance(unit);
	if (is_valid(transform)) {
		TransformInstance cur_child = scene_graph.first_child(transform);
		while (is_valid(cur_child)) {
			child_id = scene_graph.owner(cur_child);
			mesh_set_skeleton_recursively(child_id, skeleton, scene_graph, render_world);

			cur_child = scene_graph.next_sibling(cur_child);
		}
	}
}

void AnimationStateMachine::allocate(Machine &m, UnitId unit, const StateMachineResource *smr)
{
	m.unit          = unit;
	m.time          = 0.0f;
	m.time_total    = 0.0f;
	m.anim_type     = smr->animation_type;
	m.anim_resource = NULL;
	m.anim_id       = UINT32_MAX;
	m.state         = state_machine::initial_state(smr);
	m.state_next    = NULL;
	m.state_machine = smr;
	m.variables     = (f32 *)default_allocator().allocate(sizeof(*m.variables)*smr->num_variables);
	m.skeleton      = NULL;

	memcpy(m.variables, state_machine::variables(smr), sizeof(*m.variables)*smr->num_variables);

	if (smr->animation_type == RESOURCE_TYPE_MESH_ANIMATION) {
		// Spawn the skeleton hierarchy.
		const MeshSkeletonResource *skeleton_resource = (MeshSkeletonResource *)_resource_manager->get(RESOURCE_TYPE_MESH_SKELETON, smr->skeleton_name);
		const BoneTransform *local_transforms = mesh_skeleton_resource::local_transforms(skeleton_resource);
		const u32 *parents = mesh_skeleton_resource::parents(skeleton_resource);

		u32 size = sizeof(AnimationSkeletonInstance)
			+ sizeof(UnitId) * skeleton_resource->num_bones
			+ sizeof(Matrix4x4) * skeleton_resource->num_bones
			;
		AnimationSkeletonInstance *skeleton = (AnimationSkeletonInstance *)default_allocator().allocate(size);
		skeleton->num_bones = skeleton_resource->num_bones;
		skeleton->offsets = mesh_skeleton_resource::binding_matrices(skeleton_resource);
		skeleton->bone_lookup = (UnitId *)&skeleton[1];
		skeleton->bones = (Matrix4x4 *)(skeleton->bone_lookup + skeleton_resource->num_bones);
		m.skeleton = skeleton;

		for (u32 i = 0; i < skeleton_resource->num_bones; ++i)
			skeleton->bone_lookup[i] = _unit_manager->create();

		SceneGraph &scene_graph = *_world->_scene_graph;
		RenderWorld &render_world = *_world->_render_world;

		for (u32 i = 0; i < skeleton_resource->num_bones; ++i) {
			TransformInstance ti = scene_graph.create(skeleton->bone_lookup[i]
				, local_transforms[i].position
				, local_transforms[i].rotation
				, local_transforms[i].scale
				);
			if (parents[i] != UINT16_MAX) {
				TransformInstance parent_ti = scene_graph.instance(skeleton->bone_lookup[parents[i]]);
				scene_graph.link(parent_ti
					, ti
					, local_transforms[i].position
					, local_transforms[i].rotation
					, local_transforms[i].scale
					);
			} else {
				Vector3 scale = local_transforms[i].scale;
				Matrix4x4 tr = from_quaternion_translation(local_transforms[i].rotation, local_transforms[i].position);
				scene_graph.set_local_pose(ti, tr);
				scene_graph.set_local_scale(ti, scale);
			}
		}

		mesh_set_skeleton_recursively(unit, skeleton, scene_graph, render_world);
	}
}

void AnimationStateMachine::create_instances(const void *components_data
	, u32 num
	, const UnitId *unit_lookup
	, const u32 *unit_index
	)
{
	const AnimationStateMachineDesc *state_machines = (AnimationStateMachineDesc *)components_data;

	for (u32 i = 0; i < num; ++i) {
		UnitId unit = unit_lookup[unit_index[i]];
		CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a state machine component");

		const StateMachineResource *smr = (StateMachineResource *)_resource_manager->get(RESOURCE_TYPE_STATE_MACHINE, state_machines[i].state_machine_resource);

		Machine m;
		allocate(m, unit, smr);

		u32 last = array::size(_machines);
		array::push_back(_machines, m);
		hash_map::set(_map, unit, last);
	}
}

StateMachineInstance AnimationStateMachine::create(UnitId unit, const AnimationStateMachineDesc &desc)
{
	u32 unit_index = 0;
	create_instances(&desc, 1, &unit, &unit_index);
	return instance(unit);
}

void AnimationStateMachine::deallocate(Machine &m)
{
	if (m.skeleton != NULL) {
		for (u32 i = 0; i < m.skeleton->num_bones; ++i)
			_unit_manager->destroy(m.skeleton->bone_lookup[i]);
	}

	// TODO: Get rid of these allocations ASAP!
	default_allocator().deallocate(m.skeleton);
	default_allocator().deallocate(m.variables);
}

void AnimationStateMachine::destroy(StateMachineInstance state_machine)
{
	const u32 last_i = array::size(_machines) - 1;
	const UnitId u = _machines[state_machine.i].unit;
	const UnitId last_u = _machines[last_i].unit;

	deallocate(_machines[state_machine.i]);
	_machines[state_machine.i] = _machines[last_i];

	array::pop_back(_machines);
	hash_map::set(_map, last_u, state_machine.i);
	hash_map::remove(_map, u);
}

StateMachineInstance AnimationStateMachine::instance(UnitId unit)
{
	return make_instance(hash_map::get(_map, unit, UINT32_MAX));
}

bool AnimationStateMachine::has(UnitId unit)
{
	return hash_map::has(_map, unit);
}

u32 AnimationStateMachine::variable_id(StateMachineInstance state_machine, StringId32 name)
{
	const u32 index = state_machine::variable_index(_machines[state_machine.i].state_machine, name);
	return index;
}

f32 AnimationStateMachine::variable(StateMachineInstance state_machine, u32 variable_id)
{
	CE_ENSURE(variable_id != UINT32_MAX);
	return _machines[state_machine.i].variables[variable_id];
}

void AnimationStateMachine::set_variable(StateMachineInstance state_machine, u32 variable_id, f32 value)
{
	CE_ENSURE(variable_id != UINT32_MAX);
	_machines[state_machine.i].variables[variable_id] = value;
}

void AnimationStateMachine::trigger(StateMachineInstance state_machine, StringId32 event)
{
	const Transition *transition;
	const State *s = state_machine::trigger(_machines[state_machine.i].state_machine
		, _machines[state_machine.i].state
		, event
		, &transition
		);

	if (!transition)
		return;

	if (transition->mode == TransitionMode::IMMEDIATE)
		_machines[state_machine.i].state = s;
	else if (transition->mode == TransitionMode::WAIT_UNTIL_END)
		_machines[state_machine.i].state_next = s;
	else
		CE_FATAL("Unknown transition mode");
}

void AnimationStateMachine::update(float dt, SceneGraph &scene_graph)
{
	f32 stack_data[32];
	expression_language::Stack stack(stack_data, countof(stack_data));

	for (u32 ii = 0; ii < array::size(_machines); ++ii) {
		Machine &mi = _machines[ii];

		const f32 *variables = mi.variables;
		const u32 *byte_code = state_machine::byte_code(mi.state_machine);

		// Evaluate animation weights
		f32 max_v = 0.0f;
		u32 max_i = UINT32_MAX;
		StringId64 name;
		name._id = 0;

		const AnimationArray *aa = state_machine::state_animations(mi.state);
		for (u32 jj = 0; jj < aa->num; ++jj) {
			const crown::Animation *animation = state_machine::animation(aa, jj);

			stack.size = 0;
			expression_language::run(&byte_code[animation->bytecode_entry], variables, stack);
			const f32 cur = stack.size > 0 ? stack_data[stack.size - 1] : 0.0f;
			if (cur > max_v || max_i == UINT32_MAX) {
				max_v = cur;
				max_i = jj;
				name = animation->name;
			}
		}

		if (name._id == 0)
			continue;

		// Evaluate animation speed
		stack.size = 0;
		expression_language::run(&byte_code[mi.state->speed_bytecode], variables, stack);
		const f32 speed = stack.size > 0 ? stack_data[stack.size - 1] : 1.0f;

		// Advance animation.
		const void *anim_resource = _resource_manager->get(mi.anim_type, name);
		if (mi.anim_resource != anim_resource) {
			mi.anim_resource = anim_resource;
			if (mi.anim_type == RESOURCE_TYPE_MESH_ANIMATION) {
				if (mesh_animation_player::has(*_mesh_animation_player, mi.anim_id))
					mesh_animation_player::destroy(*_mesh_animation_player, mi.anim_id);
				mi.anim_id = mesh_animation_player::create(*_mesh_animation_player, (const MeshAnimationResource *)anim_resource);
				mi.time = 0.0f;
				mi.time_total = ((const MeshAnimationResource *)anim_resource)->total_time;
			} else if (mi.anim_type == RESOURCE_TYPE_SPRITE_ANIMATION) {
				if (sprite_animation_player::has(*_sprite_animation_player, mi.anim_id))
					sprite_animation_player::destroy(*_sprite_animation_player, mi.anim_id);
				mi.anim_id = sprite_animation_player::create(*_sprite_animation_player, (const SpriteAnimationResource *)anim_resource);
				mi.time = 0.0f;
				mi.time_total = ((const SpriteAnimationResource *)anim_resource)->total_time;
			}
		}

		if (!anim_resource)
			continue;

		if (mi.anim_type == RESOURCE_TYPE_MESH_ANIMATION) {
			mesh_animation_player::evaluate(*_mesh_animation_player
				, mi.anim_id
				, mi.time
				, scene_graph
				, mi.skeleton->bone_lookup
				);
		} else if (mi.anim_type == RESOURCE_TYPE_SPRITE_ANIMATION) {
			sprite_animation_player::evaluate(*_sprite_animation_player
				, mi.anim_id
				, mi.time
				, mi.unit
				, _events
				);
		}

		mi.time += dt*speed;

		// If animation finished playing
		if (mi.time > mi.time_total) {
			if (mi.state_next) {
				mi.state = mi.state_next;
				mi.state_next = NULL;
				mi.time = 0.0f;
			} else {
				if (!!mi.state->loop) {
					mi.time = fmod(mi.time, mi.time_total);
				} else {
					const Transition *dummy;
					const State *s = state_machine::trigger(mi.state_machine
						, mi.state
						, STRING_ID_32("animation_end", UINT32_C(0x119d34e1))
						, &dummy
						);
					mi.time = mi.state != s ? 0.0f : mi.time_total;
					mi.state = s;
				}
			}
		}
	}
}

void AnimationStateMachine::reload(const StateMachineResource *old_resource, const StateMachineResource *new_resource)
{
	for (u32 i = 0; i < array::size(_machines); ++i) {
		Machine &machine = _machines[i];
		UnitId unit = machine.unit;

		if (old_resource != machine.state_machine)
			continue;

		deallocate(machine);
		allocate(machine, unit, new_resource);
	}
}

void AnimationStateMachine::set_state_machine(StateMachineInstance state_machine, const StateMachineResource *state_machine_resource)
{
	Machine &machine = _machines[state_machine.i];

	deallocate(machine);
	allocate(machine, machine.unit, state_machine_resource);
}

void AnimationStateMachine::set_state_machine(StateMachineInstance state_machine, const StringId64 state_machine_name)
{
	set_state_machine(state_machine, (StateMachineResource *)_resource_manager->get(RESOURCE_TYPE_STATE_MACHINE, state_machine_name));
}

void AnimationStateMachine::unit_destroyed_callback(UnitId unit)
{
	StateMachineInstance inst = instance(unit);
	if (is_valid(inst))
		destroy(inst);
}

} // namespace crown
