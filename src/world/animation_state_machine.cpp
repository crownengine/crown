/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/types.h"
#include "core/memory/globals.h"
#include "core/strings/string_id.inl"
#include "resource/expression_language.h"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"
#include "resource/state_machine_resource.h"
#include "world/animation_state_machine.h"
#include "world/event_stream.inl"
#include "world/types.h"
#include "world/unit_manager.h"

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

AnimationStateMachine::AnimationStateMachine(Allocator &a, ResourceManager &rm, UnitManager &um)
	: _marker(ANIMATION_STATE_MACHINE_MARKER)
	, _resource_manager(&rm)
	, _unit_manager(&um)
	, _map(a)
	, _machines(a)
	, _events(a)
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
}

StateMachineInstance AnimationStateMachine::create(UnitId unit, const AnimationStateMachineDesc &desc)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has a state machine component");

	const StateMachineResource *smr = (StateMachineResource *)_resource_manager->get(RESOURCE_TYPE_STATE_MACHINE, desc.state_machine_resource);

	Machine m;
	m.unit          = unit;
	m.time          = 0.0f;
	m.time_total    = 0.0f;
	m.num_frames    = 0;
	m.frames        = NULL;
	m.resource      = NULL;
	m.state         = state_machine::initial_state(smr);
	m.state_next    = NULL;
	m.state_machine = smr;
	m.variables     = (f32 *)default_allocator().allocate(sizeof(*m.variables)*smr->num_variables);

	memcpy(m.variables, state_machine::variables(smr), sizeof(*m.variables)*smr->num_variables);

	u32 last = array::size(_machines);
	array::push_back(_machines, m);
	hash_map::set(_map, unit, last);

	return make_instance(last);
}

void AnimationStateMachine::destroy(StateMachineInstance state_machine)
{
	const u32 last_i = array::size(_machines) - 1;
	const UnitId u = _machines[state_machine.i].unit;
	const UnitId last_u = _machines[last_i].unit;

	default_allocator().deallocate(_machines[state_machine.i].variables);
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

void AnimationStateMachine::update(float dt)
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

		// Advance animation
		const SpriteAnimationResource *sar = (SpriteAnimationResource *)_resource_manager->get(RESOURCE_TYPE_SPRITE_ANIMATION, name);
		if (mi.resource != sar) {
			mi.time       = 0.0f;
			mi.time_total = sar->total_time;
			mi.num_frames = sar->num_frames;
			mi.frames     = sprite_animation_resource::frames(sar);
			mi.resource   = sar;
		}

		if (!mi.resource)
			continue;

		const f32 frame_ratio     = mi.time / mi.time_total;
		const u32 frame_unclamped = u32(frame_ratio * f32(mi.num_frames));
		const u32 frame_index     = min(frame_unclamped, mi.num_frames - 1);

		mi.time += dt*speed;

		// If animation finished playing
		if (mi.time > mi.time_total) {
			if (mi.state_next) {
				mi.state = mi.state_next;
				mi.state_next = NULL;
				mi.time = 0.0f;
			} else {
				if (!!mi.state->loop) {
					mi.time = mi.time - mi.time_total;
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

		// Emit events
		SpriteFrameChangeEvent ev;
		ev.unit      = mi.unit;
		ev.frame_num = mi.frames[frame_index];
		event_stream::write(_events, 0, ev);
	}
}

void AnimationStateMachine::unit_destroyed_callback(UnitId unit)
{
	StateMachineInstance inst = instance(unit);
	if (is_valid(inst))
		destroy(inst);
}

} // namespace crown
