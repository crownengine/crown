/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"
#include "resource/types.h"

namespace crown
{
namespace physics_resource_internal
{
	s32 compile_collider(Buffer &output, const char *json, CompileOptions &opts);
	s32 compile_actor(Buffer &output, const char *json, CompileOptions &opts);
	s32 compile_joint(Buffer &output, const char *json, CompileOptions &opts);

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

namespace physics_config_resource_internal
{
	s32 compile(CompileOptions &opts);

} // namespace physics_config_resource_internal

namespace physics_config_resource
{
	/// Returns the material @a name.
	const PhysicsMaterial *material(const PhysicsConfigResource *pcr, StringId32 name);

	/// Returns the actor @a name.
	const PhysicsActor *actor(const PhysicsConfigResource *pcr, StringId32 name);

	/// Returns the collision filter @a name.
	const PhysicsCollisionFilter *filter(const PhysicsConfigResource *pcr, StringId32 name);

} // namespace physics_config_resource

} // namespace crown
