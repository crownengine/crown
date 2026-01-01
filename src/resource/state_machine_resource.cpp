/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/vector.inl"
#include "core/error/error.h"
#include "core/filesystem/file.h"
#include "core/guid.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/math.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "resource/compile_options.inl"
#include "resource/expression_language.h"
#include "resource/state_machine_resource.h"
#include "resource/types.h"

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

#if CROWN_CAN_COMPILE
namespace state_machine_resource_internal
{
	struct TransitionModeInfo
	{
		const char *name;
		TransitionMode::Enum mode;
	};

	static TransitionModeInfo _transition_mode_map[] =
	{
		{ "immediate",      TransitionMode::IMMEDIATE      },
		{ "wait_until_end", TransitionMode::WAIT_UNTIL_END },
	};
	CE_STATIC_ASSERT(countof(_transition_mode_map) == TransitionMode::COUNT);

	static TransitionMode::Enum name_to_transition_mode(const char *name)
	{
		for (u32 i = 0; i < countof(_transition_mode_map); ++i) {
			if (strcmp(name, _transition_mode_map[i].name) == 0)
				return _transition_mode_map[i].mode;
		}

		return TransitionMode::COUNT;
	}

	struct OffsetAccumulator
	{
		u32 _offset;

		OffsetAccumulator()
			: _offset(sizeof(StateMachineResource))
		{
		}

		void align(u32 align)
		{
			_offset = (u32)(uintptr_t)memory::align_top((void *)(uintptr_t)_offset, align);
		}

		// Returns the offset of
		u32 offset(u32 num_animations, u32 num_transitions)
		{
			const u32 offt = _offset;
			align(alignof(State));
			_offset += sizeof(State);
			align(alignof(Transition));
			_offset += sizeof(Transition) * num_transitions;
			align(alignof(AnimationArray));
			_offset += sizeof(AnimationArray);
			align(alignof(Animation));
			_offset += sizeof(Animation) * num_animations;
			return offt;
		}
	};

	struct AnimationInfo
	{
		ALLOCATOR_AWARE;

		StringId64 name;
		DynamicString weight;
		u32 bytecode_entry;

		explicit AnimationInfo(Allocator &a)
			: weight(a)
		{
		}
	};

	struct TransitionInfo
	{
		Transition transition;
		Guid state;
	};

	struct StateInfo
	{
		ALLOCATOR_AWARE;

		Vector<AnimationInfo> animations;
		Vector<TransitionInfo> transitions;
		DynamicString speed;
		u32 speed_bytecode;
		u32 loop;

		explicit StateInfo(Allocator &a)
			: animations(a)
			, transitions(a)
			, speed(a)
			, speed_bytecode(UINT32_MAX)
			, loop(0)
		{
		}
	};

	struct VariableInfo
	{
		ALLOCATOR_AWARE;

		DynamicString name_string;
		StringId32 name;
		float value;

		explicit VariableInfo(Allocator &a)
			: name_string(a)
			, value(0.0f)
		{
		}
	};

	struct StateMachineCompiler
	{
		CompileOptions &_opts;
		Guid _initial_state;
		HashMap<Guid, StateInfo> _states;
		OffsetAccumulator _offset_accumulator;
		HashMap<Guid, u32> _offsets;
		Vector<VariableInfo> _variables;
		Array<u32> _byte_code;
		DynamicString _animation_type;
		StringId64 _skeleton_name;

		explicit StateMachineCompiler(CompileOptions &opts)
			: _opts(opts)
			, _states(default_allocator())
			, _offsets(default_allocator())
			, _variables(default_allocator())
			, _byte_code(default_allocator())
			, _animation_type(default_allocator())
		{
		}

		s32 parse_animations(StateInfo &si, const JsonArray &animations)
		{
			for (u32 i = 0; i < array::size(animations); ++i) {
				TempAllocator4096 ta;
				JsonObject animation(ta);
				RETURN_IF_ERROR(sjson::parse_object(animation, animations[i]), _opts);

				if (json_object::has(animation, "name")
					&& sjson::type(animation["name"]) == JsonValueType::STRING) {
					DynamicString animation_resource(ta);
					RETURN_IF_ERROR(sjson::parse_string(animation_resource, animation["name"]), _opts);
					RETURN_IF_RESOURCE_MISSING(_animation_type.c_str()
						, animation_resource.c_str()
						, _opts
						);
					_opts.add_requirement(_animation_type.c_str(), animation_resource.c_str());

					AnimationInfo ai(ta);
					ai.name = RETURN_IF_ERROR(sjson::parse_resource_name(animation["name"]), _opts);
					RETURN_IF_ERROR(sjson::parse_string(ai.weight, animation["weight"]), _opts);

					vector::push_back(si.animations, ai);
				}
			}

			return 0;
		}

		s32 parse_transitions(StateInfo &si, const JsonArray &transitions)
		{
			for (u32 i = 0; i < array::size(transitions); ++i) {
				TempAllocator4096 ta;
				JsonObject transition(ta);
				RETURN_IF_ERROR(sjson::parse_object(transition, transitions[i]), _opts);

				DynamicString mode_str(ta);
				RETURN_IF_ERROR(sjson::parse_string(mode_str, transition["mode"]), _opts);
				const u32 mode = name_to_transition_mode(mode_str.c_str());
				RETURN_IF_FALSE(mode != TransitionMode::COUNT
					, _opts
					, "Unknown transition mode: '%s'"
					, mode_str.c_str()
					);

				TransitionInfo ti;
				ti.transition.event        = RETURN_IF_ERROR(sjson::parse_string_id(transition["event"]), _opts);
				ti.transition.state_offset = UINT32_MAX;
				ti.transition.mode         = mode;
				ti.state                   = RETURN_IF_ERROR(sjson::parse_guid(transition["to"]), _opts);

				vector::push_back(si.transitions, ti);
			}

			return 0;
		}

		s32 parse_states(const JsonArray &states)
		{
			for (u32 i = 0; i < array::size(states); ++i) {
				TempAllocator4096 ta;
				JsonObject state(ta);
				JsonArray animations(ta);
				JsonArray transitions(ta);
				RETURN_IF_ERROR(sjson::parse_object(state, states[i]), _opts);
				RETURN_IF_ERROR(sjson::parse_array(animations, state["animations"]), _opts);
				RETURN_IF_ERROR(sjson::parse_array(transitions, state["transitions"]), _opts);

				StateInfo si(ta);
				RETURN_IF_ERROR(sjson::parse_string(si.speed, state["speed"]), _opts);
				si.loop = RETURN_IF_ERROR(sjson::parse_bool(state["loop"]), _opts);

				s32 err = 0;
				err = parse_transitions(si, transitions);
				ENSURE_OR_RETURN(err == 0, _opts);
				err = parse_animations(si, animations);
				ENSURE_OR_RETURN(err == 0, _opts);

				Guid guid;
				if (json_object::has(state, "id")) {
					guid = RETURN_IF_ERROR(sjson::parse_guid(state["id"]), _opts);
				} else {
					guid = RETURN_IF_ERROR(sjson::parse_guid(state["_guid"]), _opts);
				}

				RETURN_IF_FALSE(!hash_map::has(_states, guid)
					, _opts
					, "State GUID duplicated"
					);
				hash_map::set(_states, guid, si);
			}

			return 0;
		}

		s32 parse_variables(const JsonArray &variables)
		{
			for (u32 i = 0; i < array::size(variables); ++i) {
				TempAllocator4096 ta;
				JsonObject variable(ta);
				RETURN_IF_ERROR(sjson::parse_object(variable, variables[i]), _opts);

				VariableInfo vi(ta);
				vi.name  = RETURN_IF_ERROR(sjson::parse_string_id(variable["name"]), _opts);
				vi.value = RETURN_IF_ERROR(sjson::parse_float(variable["value"]), _opts);
				RETURN_IF_ERROR(sjson::parse_string(vi.name_string, variable["name"]), _opts);

				vector::push_back(_variables, vi);
			}

			return 0;
		}

		s32 compute_state_offsets()
		{
			// Limit byte code to 4K
			array::resize(_byte_code, 1024);
			u32 written = 0;

			const u32 num_variables = vector::size(_variables);
			const char **variables = (const char **)default_allocator().allocate(num_variables*sizeof(char *));

			for (u32 i = 0; i < num_variables; ++i)
				variables[i] = _variables[i].name_string.c_str();

			const u32 num_constants = 1;
			const char *constants[] =
			{
				"PI"
			};
			const f32 constant_values[] =
			{
				PI
			};

			auto cur = hash_map::begin(_states);
			auto end = hash_map::end(_states);
			for (; cur != end; ++cur) {
				HASH_MAP_SKIP_HOLE(_states, cur);

				const Guid &guid    = cur->first;
				const StateInfo &si = cur->second;

				const u32 offset = _offset_accumulator.offset(vector::size(si.animations), vector::size(si.transitions));
				hash_map::set(_offsets, guid, offset);

				for (u32 i = 0; i < vector::size(si.animations); ++i) {
					const u32 num = expression_language::compile(si.animations[i].weight.c_str()
						, num_variables
						, variables
						, num_constants
						, constants
						, constant_values
						, array::begin(_byte_code) + written
						, array::size(_byte_code)
						);

					const_cast<AnimationInfo &>(si.animations[i]).bytecode_entry = num > 0 ? written : UINT32_MAX;
					written += num;
				}

				const u32 num = expression_language::compile(si.speed.c_str()
					, num_variables
					, variables
					, num_constants
					, constants
					, constant_values
					, array::begin(_byte_code) + written
					, array::size(_byte_code)
					);

				const_cast<StateInfo &>(si).speed_bytecode = num > 0 ? written : UINT32_MAX;
				written += num;
			}

			// Resize to total amount of written bytecode
			array::resize(_byte_code, written);

			default_allocator().deallocate(variables);

			return 0;
		}

		s32 parse(Buffer &buf)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			JsonArray states(ta);
			JsonArray variables(ta);

			RETURN_IF_ERROR(sjson::parse(obj, buf), _opts);
			RETURN_IF_ERROR(sjson::parse_array(states, obj["states"]), _opts);
			RETURN_IF_ERROR(sjson::parse_array(variables, obj["variables"]), _opts);

			// Parse animation type.
			if (json_object::has(obj, "animation_type")) {
				RETURN_IF_ERROR(sjson::parse_string(_animation_type, obj["animation_type"]), _opts);
			} else {
				_animation_type = "sprite_animation"; // For backwards compatibility.
			}
			RETURN_IF_FALSE(StringId64(_animation_type.c_str()) == RESOURCE_TYPE_SPRITE_ANIMATION
				|| StringId64(_animation_type.c_str()) == RESOURCE_TYPE_MESH_ANIMATION
				, _opts
				, "Unknown animation type '%s'"
				, _animation_type.c_str()
				);

			s32 err = 0;
			err = parse_states(states);
			ENSURE_OR_RETURN(err == 0, _opts);
			RETURN_IF_FALSE(hash_map::size(_states) > 0
				, _opts
				, "States cannot be empty"
				);

			_initial_state = RETURN_IF_ERROR(sjson::parse_guid(obj["initial_state"]), _opts);
			RETURN_IF_FALSE(hash_map::has(_states, _initial_state)
				, _opts
				, "Initial state references non-existing state"
				);

			// Parse skeleton, if any.
			if (json_object::has(obj, "skeleton_name")
				&& sjson::type(obj["skeleton_name"]) == JsonValueType::STRING) {
				DynamicString skeleton_name(ta);
				RETURN_IF_ERROR(sjson::parse_string(skeleton_name, obj["skeleton_name"]), _opts);
				RETURN_IF_RESOURCE_MISSING("mesh_skeleton", skeleton_name.c_str(), _opts);
				_opts.add_requirement("mesh_skeleton", skeleton_name.c_str());
				_skeleton_name = StringId64(skeleton_name.c_str());
			}

			err = parse_variables(variables);
			ENSURE_OR_RETURN(err == 0, _opts);

			err = compute_state_offsets();
			ENSURE_OR_RETURN(err == 0, _opts);

			return 0;
		}

		s32 write()
		{
			StateMachineResource smr;
			smr.version = RESOURCE_HEADER(RESOURCE_VERSION_STATE_MACHINE);
			smr.initial_state_offset = hash_map::get(_offsets, _initial_state, UINT32_MAX);
			smr.num_variables = vector::size(_variables);
			smr.variables_offset = _offset_accumulator._offset; // Offset of last state + 1
			smr.bytecode_size = array::size(_byte_code)*4;
			smr.bytecode_offset = smr.variables_offset + smr.num_variables*4*2;
			smr.animation_type = StringId64(_animation_type.c_str());
			smr.skeleton_name = _skeleton_name;
			_opts.write(smr.version);
			_opts.write(smr.initial_state_offset);
			_opts.write(smr.num_variables);
			_opts.write(smr.variables_offset);
			_opts.write(smr.bytecode_size);
			_opts.write(smr.bytecode_offset);
			_opts.write(smr.animation_type);
			_opts.write(smr.skeleton_name);

			// Write states
			auto cur = hash_map::begin(_states);
			auto end = hash_map::end(_states);
			for (; cur != end; ++cur) {
				HASH_MAP_SKIP_HOLE(_states, cur);

				const StateInfo &si = cur->second;
				const u32 num_animations  = vector::size(si.animations);
				const u32 num_transitions = vector::size(si.transitions);

				// Write speed
				_opts.write(si.speed_bytecode);

				// Write loop
				_opts.write(si.loop);

				// Write transitions
				TransitionArray ta;
				ta.num = num_transitions;
				_opts.write(ta.num);
				for (u32 i = 0; i < num_transitions; ++i) {
					Transition t = si.transitions[i].transition;
					t.state_offset = hash_map::get(_offsets, si.transitions[i].state, UINT32_MAX);

					_opts.write(t.event);
					_opts.write(t.state_offset);
					_opts.write(t.mode);
				}

				// Write animations
				AnimationArray aa;
				aa.num = num_animations;
				_opts.write(aa.num);
				for (u32 i = 0; i < num_animations; ++i) {
					Animation a;
					a.name = si.animations[i].name;
					a.bytecode_entry = si.animations[i].bytecode_entry;
					a.pad = 0;

					_opts.write(a.name);
					_opts.write(a.bytecode_entry);
					_opts.write(a.pad);
				}
			}

			// Write variables
			for (u32 i = 0; i < vector::size(_variables); ++i)
				_opts.write(_variables[i].name);
			for (u32 i = 0; i < vector::size(_variables); ++i)
				_opts.write(_variables[i].value);

			// Write bytecode
			for (u32 i = 0; i < array::size(_byte_code); ++i)
				_opts.write(_byte_code[i]);

			return 0;
		}
	};

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();

		StateMachineCompiler smc(opts);
		s32 err = 0;
		err = smc.parse(buf);
		ENSURE_OR_RETURN(err == 0, opts);

		return smc.write();
	}

} // namespace state_machine_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
