/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/json/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"

namespace crown
{
namespace physics_resource_internal
{
	s32 compile_collider(Buffer &output, FlatJsonObject &obj, CompileOptions &opts);
	s32 compile_actor(Buffer &output, FlatJsonObject &obj, CompileOptions &opts);
	s32 compile_mover(Buffer &output, FlatJsonObject &obj, CompileOptions &opts);
	s32 compile_joint(Buffer &output, FlatJsonObject &obj, CompileOptions &opts);

} // namespace physics_resource_internal

struct PhysicsConfigResource
{
	u32 version;
	u32 num_materials;
	u32 materials_offset;
	u32 num_actors;
	u32 actors_offset;
	u32 num_filters;
	u32 filters_offset;
	Vector3 gravity;
};

struct PhysicsMaterial
{
	StringId32 name;
	f32 friction;
	f32 rolling_friction;
	f32 spinning_friction;
	f32 restitution;
};

struct PhysicsCollisionFilter
{
	StringId32 name;
	u32 me;
	u32 mask;
};

#define CROWN_PHYSICS_ACTOR_DYNAMIC         UINT32_C(0x1)
#define CROWN_PHYSICS_ACTOR_KINEMATIC       UINT32_C(0x2)
#define CROWN_PHYSICS_ACTOR_DISABLE_GRAVITY UINT32_C(0x4)
#define CROWN_PHYSICS_ACTOR_TRIGGER         UINT32_C(0x8)

struct PhysicsActor
{
	StringId32 name;
	f32 linear_damping;
	f32 angular_damping;
	u32 flags;
};

namespace physics_config_resource
{
	/// Returns the material at @a index.
	const PhysicsMaterial *materials_array(const PhysicsConfigResource *pcr);

	/// Returns the index of the material @a name.
	u32 material_index(const PhysicsMaterial *materials, u32 num, StringId32 name);

	/// Returns the actor at @a index.
	const PhysicsActor *actors_array(const PhysicsConfigResource *pcr);

	/// Returns the index of the actor @a name.
	u32 actor_index(const PhysicsActor *actors, u32 num, StringId32 name);

	/// Returns the collision filter at @a index.
	const PhysicsCollisionFilter *filters_array(const PhysicsConfigResource *pcr);

	/// Returns the index of the collision filter @a name.
	u32 filter_index(const PhysicsCollisionFilter *filters, u32 num, StringId32 name);

} // namespace physics_config_resource

} // namespace crown
