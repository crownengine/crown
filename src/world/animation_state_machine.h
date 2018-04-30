/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/event_stream.h"
#include "core/containers/types.h"
#include "resource/state_machine_resource.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
struct SpriteFrameChangeEvent
{
	UnitId unit;
	u32 frame_num;
};

struct AnimationStateMachine
{
	struct Animation
	{
		UnitId unit;
		f32 time_total;
		f32 time;
		u32 num_frames;
		const u32* frames;
		const SpriteAnimationResource* resource;
		const State* state;
		const State* state_next;
		const StateMachineResource* state_machine;
		f32* variables;
	};

	u32 _marker;
	ResourceManager* _resource_manager;
	UnitManager* _unit_manager;
	HashMap<UnitId, u32> _map;
	Array<Animation> _animations;
	EventStream _events;

	///
	AnimationStateMachine(Allocator& a, ResourceManager& rm, UnitManager& um);

	///
	~AnimationStateMachine();

	///
	u32 create(UnitId unit, const AnimationStateMachineDesc& desc);

	///
	void destroy(UnitId unit);

	///
	u32 instances(UnitId unit);

	///
	bool has(UnitId unit);

	///
	u32 variable_id(UnitId unit, StringId32 name);

	///
	f32 variable(UnitId unit, u32 variable_id);

	///
	void set_variable(UnitId unit, u32 variable_id, f32 value);

	///
	void trigger(UnitId unit, StringId32 event);

	///
	void update(float dt);

	///
	void unit_destroyed_callback(UnitId unit);
};

} // namespace crown
