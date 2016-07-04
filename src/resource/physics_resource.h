/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compiler_types.h"
#include "container_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "string_id.h"

namespace crown
{
namespace physics_resource
{
	inline void compile(const char* /*path*/, CompileOptions& /*opts*/) {}
	inline void* load(File& /*file*/, Allocator& /*a*/) { return NULL; }
	inline void unload(Allocator& /*a*/, void* /*res*/) {}
	Buffer compile_controller(const char* json, CompileOptions& opts);
	Buffer compile_collider(const char* json, CompileOptions& opts);
	Buffer compile_actor(const char* json, CompileOptions& opts);
	Buffer compile_joint(const char* json, CompileOptions& opts);
} // namespace physics_resource

struct PhysicsConfigResource
{
	u32 version;
	u32 num_materials;
	u32 materials_offset;
	u32 num_shapes;
	u32 shapes_offset;
	u32 num_actors;
	u32 actors_offset;
	u32 num_filters;
	u32 filters_offset;
};

struct PhysicsConfigMaterial
{
	StringId32 name;
	f32 static_friction;
	f32 dynamic_friction;
	f32 restitution;
};

struct PhysicsCollisionFilter
{
	StringId32 name;
	u32 me;
	u32 mask;
};

struct PhysicsConfigShape
{
	StringId32 name;
	bool trigger;
	char _pad[3];
};

struct PhysicsConfigActor
{
	enum
	{
		DYNAMIC         = 1 << 0,
		KINEMATIC       = 1 << 1,
		DISABLE_GRAVITY = 1 << 2
	};

	StringId32 name;
	f32 linear_damping;
	f32 angular_damping;
	u32 flags;
};

namespace physics_config_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);

	const PhysicsConfigMaterial* material(const PhysicsConfigResource* pcr, StringId32 name);
	const PhysicsConfigShape* shape(const PhysicsConfigResource* pcr, StringId32 name);
	const PhysicsConfigActor* actor(const PhysicsConfigResource* pcr, StringId32 name);
	const PhysicsCollisionFilter* filter(const PhysicsConfigResource* pcr, StringId32 name);
} // namespace physics_resource
} // namespace crown
