/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "compiler_types.h"
#include "string_id.h"
#include "container_types.h"

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
	uint32_t version;
	uint32_t num_materials;
	uint32_t materials_offset;
	uint32_t num_shapes;
	uint32_t shapes_offset;
	uint32_t num_actors;
	uint32_t actors_offset;
	uint32_t num_filters;
	uint32_t filters_offset;
};

struct PhysicsConfigMaterial
{
	StringId32 name;
	float static_friction;
	float dynamic_friction;
	float restitution;
};

struct PhysicsCollisionFilter
{
	StringId32 name;
	uint32_t me;
	uint32_t mask;
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
		DYNAMIC         = (1 << 0),
		KINEMATIC       = (1 << 1),
		DISABLE_GRAVITY = (1 << 2)
	};

	StringId32 name;
	float linear_damping;
	float angular_damping;
	uint32_t flags;
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
