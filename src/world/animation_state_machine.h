/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "resource/state_machine_resource.h"
#include "resource/types.h"
#include "world/event_stream.h"
#include "world/mesh_animation_player.h"
#include "world/sprite_animation_player.h"
#include "world/scene_graph.h"
#include "world/types.h"

namespace crown
{
struct AnimationStateMachine
{
	struct Machine
	{
		UnitId unit;
		f32 time_total;
		f32 time;
		StringId64 anim_type;
		const void *anim_resource;
		AnimationId anim_id;
		const State *state;
		const State *state_next;
		const StateMachineResource *state_machine;
		f32 *variables;
		AnimationSkeletonInstance *skeleton;
	};

	u32 _marker;
	ResourceManager *_resource_manager;
	UnitManager *_unit_manager;
	HashMap<UnitId, u32> _map;
	Array<Machine> _machines;
	EventStream _events;
	UnitDestroyCallback _unit_destroy_callback;
	SpriteAnimationPlayer *_sprite_animation_player;
	MeshAnimationPlayer *_mesh_animation_player;
	World *_world;

	///
	AnimationStateMachine(Allocator &a
		, ResourceManager &rm
		, UnitManager &um
		, SpriteAnimationPlayer &sprite_player
		, MeshAnimationPlayer &mesh_player
		, World &world
		);

	///
	~AnimationStateMachine();

	///
	void allocate(Machine &m, UnitId unit, const StateMachineResource *smr);

	///
	void deallocate(Machine &m);

	///
	void create_instances(const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Creates a new state machine instance for the @a unit.
	StateMachineInstance create(UnitId unit, const AnimationStateMachineDesc &desc);

	/// Destroys the @a state_machine.
	void destroy(StateMachineInstance state_machine);

	/// Returns the ID of the state machine owned by the *unit*.
	StateMachineInstance instance(UnitId unit);

	/// Returns whether the @a unit has the state machine.
	bool has(UnitId unit);

	/// Returns the ID of the variable @a name in the @a state_machine.
	u32 variable_id(StateMachineInstance state_machine, StringId32 name);

	/// Returns the value of the @a variable_id in the @a state_machine.
	f32 variable(StateMachineInstance state_machine, u32 variable_id);

	/// Sets the @a value of the @a variable_id in the @a state_machine.
	void set_variable(StateMachineInstance state_machine, u32 variable_id, f32 value);

	// Triggers the @a event in the @a state_machine.
	void trigger(StateMachineInstance state_machine, StringId32 event);

	/// Sets the @a state_machine_resource of @a state_machine.
	void set_state_machine(StateMachineInstance state_machine, const StateMachineResource *state_machine_resource);

	/// Sets the @a state_machine_resource of @a state_machine.
	void set_state_machine(StateMachineInstance state_machine, StringId64 state_machine_name);

	///
	void update(float dt, SceneGraph &scene_graph);

	///
	void unit_destroyed_callback(UnitId unit);

	///
	void reload(const StateMachineResource *old_resource, const StateMachineResource *new_resource);
};

} // namespace crown
