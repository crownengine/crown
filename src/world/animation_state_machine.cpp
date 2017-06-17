/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "core/containers/array.h"
#include "core/containers/hash_map.h"
#include "core/containers/types.h"
#include "resource/expression_language.h"
#include "resource/resource_manager.h"
#include "resource/sprite_resource.h"
#include "resource/state_machine_resource.h"
#include "world/animation_state_machine.h"
#include "world/types.h"
#include "world/unit_manager.h"

namespace crown
{
static void unit_destroyed_callback_bridge(UnitId id, void* user_ptr)
{
	((AnimationStateMachine*)user_ptr)->unit_destroyed_callback(id);
}

AnimationStateMachine::AnimationStateMachine(Allocator& a, ResourceManager& rm, UnitManager& um)
	: _marker(ANIMATION_STATE_MACHINE_MARKER)
	, _resource_manager(&rm)
	, _unit_manager(&um)
	, _map(a)
	, _animations(a)
	, _events(a)
{
	um.register_destroy_function(unit_destroyed_callback_bridge, this);
}

AnimationStateMachine::~AnimationStateMachine()
{
	_unit_manager->unregister_destroy_function(this);
	_marker = 0;
}

u32 AnimationStateMachine::create(UnitId unit, const AnimationStateMachineDesc& desc)
{
	CE_ASSERT(!hash_map::has(_map, unit), "Unit already has this component");

	const StateMachineResource* smr = (StateMachineResource*)_resource_manager->get(RESOURCE_TYPE_STATE_MACHINE, desc.state_machine_resource);

	Animation anim;
	anim.unit          = unit;
	anim.time          = 0.0f;
	anim.time_total    = 0.0f;
	anim.num_frames    = 0;
	anim.frames        = NULL;
	anim.resource      = NULL;
	anim.state         = state_machine::initial_state(smr);
	anim.state_machine = smr;
	anim.variables     = (f32*)default_allocator().allocate(sizeof(*anim.variables)*smr->num_variables);

	memcpy(anim.variables, state_machine::variables(smr), sizeof(*anim.variables)*smr->num_variables);

	u32 last = array::size(_animations);
	array::push_back(_animations, anim);
	hash_map::set(_map, unit, last);
	return 0;
}

void AnimationStateMachine::destroy(UnitId unit)
{
	const u32 i = hash_map::get(_map, unit, UINT32_MAX);
	const u32 last_i = array::size(_animations) - 1;
	const UnitId last_u = _animations[last_i].unit;

	default_allocator().deallocate(_animations[i].variables);
	_animations[i] = _animations[last_i];

	array::pop_back(_animations);
	hash_map::set(_map, last_u, i);
	hash_map::remove(_map, unit);
}

u32 AnimationStateMachine::instances(UnitId unit)
{
	return hash_map::get(_map, unit, UINT32_MAX);
}

bool AnimationStateMachine::has(UnitId unit)
{
	return hash_map::has(_map, unit);
}

u32 AnimationStateMachine::variable_id(UnitId unit, StringId32 name)
{
	const u32 i = hash_map::get(_map, unit, UINT32_MAX);
	const u32 index = state_machine::variable_index(_animations[i].state_machine, name);
	return index;
}

f32 AnimationStateMachine::variable(UnitId unit, u32 variable_id)
{
	const u32 i = hash_map::get(_map, unit, UINT32_MAX);
	CE_ENSURE(variable_id != UINT32_MAX);
	return _animations[i].variables[variable_id];
}

void AnimationStateMachine::set_variable(UnitId unit, u32 variable_id, f32 value)
{
	const u32 i = hash_map::get(_map, unit, UINT32_MAX);
	CE_ENSURE(variable_id != UINT32_MAX);
	_animations[i].variables[variable_id] = value;
}

void AnimationStateMachine::trigger(UnitId unit, StringId32 event)
{
	const u32 i = hash_map::get(_map, unit, UINT32_MAX);
	Animation& anim = _animations[i];

	const TransitionArray* ta = state_machine::state_transitions(anim.state);

	for (u32 i = 0; i < ta->num; ++i)
	{
		const Transition* transition_i = state_machine::transition(ta, i);

		if (transition_i->event == event)
		{
			anim.state = state_machine::state(anim.state_machine, transition_i);
			return;
		}
	}
}

void AnimationStateMachine::update(float dt)
{
	for (u32 i = 0; i < array::size(_animations); ++i)
	{
		Animation& anim_i = _animations[i];

		f32 stack_data[32];
		const f32* variables = anim_i.variables;
		const u32* byte_code = state_machine::byte_code(anim_i.state_machine);

		// Evaluate animation weights
		f32 max_v = 0.0f;
		u32 max_i = UINT32_MAX;
		StringId64 name;

		const AnimationArray* aa = state_machine::state_animations(anim_i.state);
		for (u32 i = 0; i < aa->num; ++i)
		{
			const crown::Animation* animation = state_machine::animation(aa, i);

			skinny::expression_language::Stack stack(stack_data, countof(stack_data));
			skinny::expression_language::run(&byte_code[animation->bytecode_entry], variables, stack);
			CE_ENSURE(stack.size > 0);
			const f32 cur = stack_data[stack.size-1];
			if (cur > max_v || max_i == UINT32_MAX)
			{
				max_v = cur;
				max_i = i;
				name = animation->name;
			}
		}

		// Evaluate speed
		f32 speed = 1.0f;
		skinny::expression_language::Stack stack(stack_data, countof(stack_data));
		skinny::expression_language::run(&byte_code[anim_i.state->speed_bytecode], variables, stack);
		CE_ENSURE(stack.size > 0);
		speed = stack_data[stack.size-1];

		// Play animation
		const SpriteAnimationResource* sar = (SpriteAnimationResource*)_resource_manager->get(RESOURCE_TYPE_SPRITE_ANIMATION, name);
		if (anim_i.resource != sar)
		{
			anim_i.time       = 0.0f;
			anim_i.time_total = sar->total_time;
			anim_i.num_frames = sar->num_frames;
			anim_i.frames     = sprite_animation_resource::frames(sar);
			anim_i.resource   = sar;
		}

		if (!anim_i.resource)
			continue;

		f32 frame_time  = f32(anim_i.num_frames) * (anim_i.time/anim_i.time_total);
		u32 frame_index = u32(frame_time) % anim_i.num_frames;

		anim_i.time += dt*speed;
		if (anim_i.time > anim_i.time_total)
			anim_i.time = anim_i.time - anim_i.time_total;

		// Emit events
		SpriteFrameChangeEvent ev;
		ev.unit      = anim_i.unit;
		ev.frame_num = anim_i.frames[frame_index];
		event_stream::write(_events, 0, ev);
	}
}

void AnimationStateMachine::unit_destroyed_callback(UnitId unit)
{
	if (has(unit))
		destroy(unit);
}

} // namespace crown
