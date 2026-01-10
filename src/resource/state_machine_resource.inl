/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/state_machine_resource.h"

namespace crown
{
namespace state_machine
{
	const State *initial_state(const StateMachineResource *smr)
	{
		return (State *)((char *)smr + smr->initial_state_offset);
	}

	const State *state(const StateMachineResource *smr, const Transition *t)
	{
		return (State *)((char *)smr + t->state_offset);
	}

	const State *trigger(const StateMachineResource *smr, const State *s, StringId32 event, const Transition **transition_out)
	{
		const TransitionArray *ta = state_transitions(s);

		for (u32 i = 0; i < ta->num; ++i) {
			const Transition *transition_i = transition(ta, i);

			if (transition_i->event == event) {
				*transition_out = transition_i;
				return state(smr, transition_i);
			}
		}

		*transition_out = NULL;
		return s;
	}

	const TransitionArray *state_transitions(const State *s)
	{
		return &s->ta;
	}

	const Transition *transition(const TransitionArray *ta, u32 index)
	{
		CE_ASSERT(index < ta->num, "Index out of bounds");
		const Transition *first = (Transition *)(&ta[1]);
		return &first[index];
	}

	const AnimationArray *state_animations(const State *s)
	{
		const TransitionArray *ta = state_transitions(s);
		const Transition *first = (Transition *)(&ta[1]);
		return (AnimationArray *)(first + ta->num);
	}

	const Animation *animation(const AnimationArray *aa, u32 index)
	{
		CE_ASSERT(index < aa->num, "Index out of bounds");
		Animation *first = (Animation *)(memory::align_top((void *)&aa[1], alignof(Animation)));
		return &first[index];
	}

	static inline const StringId32 *variables_names(const StateMachineResource *smr)
	{
		return (StringId32 *)((char *)smr + smr->variables_offset);
	}

	const f32 *variables(const StateMachineResource *smr)
	{
		const StringId32 *names = variables_names(smr);
		return (f32 *)(names + smr->num_variables);
	}

	u32 variable_index(const StateMachineResource *smr, StringId32 name)
	{
		const StringId32 *names = variables_names(smr);
		for (u32 i = 0; i < smr->num_variables; ++i) {
			if (names[i] == name)
				return i;
		}

		return UINT32_MAX;
	}

	const u32 *byte_code(const StateMachineResource *smr)
	{
		return (u32 *)((char *)smr + smr->bytecode_offset);
	}

} // namespace state_machine

} // namespace crown
