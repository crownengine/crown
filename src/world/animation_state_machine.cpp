/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
	anim.state_next    = NULL;
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

	const Transition* transition;
	const State* s = state_machine::trigger(_animations[i].state_machine
		, _animations[i].state
		, event
		, &transition
		);

	if (!transition)
		return;

	if (transition->mode == TransitionMode::IMMEDIATE)
		_animations[i].state = s;
	else if (transition->mode == TransitionMode::WAIT_UNTIL_END)
		_animations[i].state_next = s;
	else
		CE_FATAL("Unknown transition mode");
}

void AnimationStateMachine::update(float dt)
{
	f32 stack_data[32];
	skinny::expression_language::Stack stack(stack_data, countof(stack_data));

	for (u32 i = 0; i < array::size(_animations); ++i)
	{
		Animation& anim_i = _animations[i];

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

			stack.size = 0;
			skinny::expression_language::run(&byte_code[animation->bytecode_entry], variables, stack);
			const f32 cur = stack.size > 0 ? stack_data[stack.size-1] : 0.0f;
			if (cur > max_v || max_i == UINT32_MAX)
			{
				max_v = cur;
				max_i = i;
				name = animation->name;
			}
		}

		// Evaluate animation speed
		stack.size = 0;
		skinny::expression_language::run(&byte_code[anim_i.state->speed_bytecode], variables, stack);
		const f32 speed = stack.size > 0 ? stack_data[stack.size-1] : 1.0f;

		// Advance animation
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

		const f32 frame_time  = f32(anim_i.num_frames) * (anim_i.time/anim_i.time_total);
		const u32 frame_index = u32(frame_time) % anim_i.num_frames;

		anim_i.time += dt*speed;

		// If animation finished playing
		if (anim_i.time > anim_i.time_total)
		{
			if (anim_i.state_next)
			{
				anim_i.state = anim_i.state_next;
				anim_i.state_next = NULL;
				anim_i.time = 0.0f;
			}
			else
			{
				if (!!anim_i.state->loop)
				{
					anim_i.time = anim_i.time - anim_i.time_total;
				}
				else
				{
					const Transition* dummy;
					const State* s = state_machine::trigger(anim_i.state_machine
						, anim_i.state
						, StringId32("animation_end")
						, &dummy
						);
					anim_i.time = anim_i.state != s ? 0.0f : anim_i.time_total;
					anim_i.state = s;
				}
			}
		}

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
