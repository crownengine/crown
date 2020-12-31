/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"

namespace crown
{
struct StateMachineResource
{
	u32 version;
	u32 initial_state_offset;
	u32 num_variables;
	u32 variables_offset;
	u32 bytecode_size;
	u32 bytecode_offset;
	// State[...]
	// StringId32[num_variables]
	// f32[num_variables]
	// u32 bytecode[bytecode_size]
};

struct AnimationArray
{
	u32 num;
};

struct Animation
{
	StringId64 name;
	u32 bytecode_entry;
	u32 pad;
};

struct TransitionArray
{
	u32 num;
};

struct Transition
{
	StringId32 event;
	u32 state_offset;
	u32 mode;         // TransitionMode::Enum
};

struct State
{
	u32 speed_bytecode;
	u32 loop;
	TransitionArray ta;
	// Transition[num_transitions]
	// AnimationArray
	// Animation[num_animations]
};

struct TransitionMode
{
	enum Enum
	{
		IMMEDIATE,
		WAIT_UNTIL_END,

		COUNT
	};
};

namespace state_machine_internal
{
	s32 compile(CompileOptions& opts);

} // namespace state_machine_internal

namespace state_machine
{
	/// Returns the initial state.
	const State* initial_state(const StateMachineResource* smr);

	/// Returns the state pointed to by transition @a t.
	const State* state(const StateMachineResource* smr, const Transition* t);

	/// Triggers the transition *event* in the state *s* and returns the resulting state.
	/// If no transition with *event* is found it returns the state *s*.
	const State* trigger(const StateMachineResource* smr, const State* s, StringId32 event, const Transition** transition_out);

	/// Returns the transitions for the state @a s.
	const TransitionArray* state_transitions(const State* s);

	/// Returns the transition with given @a index.
	const Transition* transition(const TransitionArray* ta, u32 index);

	/// Returns the animations for the state @a s.
	const AnimationArray* state_animations(const State* s);

	/// Returns the animation with the given @a index.
	const Animation* animation(const AnimationArray* aa, u32 index);

	/// Returns the variables of the state machine.
	const f32* variables(const StateMachineResource* smr);

	/// Returns the index of the variable @a name or UINT32_MAX if not found.
	u32 variable_index(const StateMachineResource* smr, StringId32 name);

	/// Returns the byte code of the state machine.
	const u32* byte_code(const StateMachineResource* smr);

} // namespace state_machine

} // namespace crown
