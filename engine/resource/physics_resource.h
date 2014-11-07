/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "compiler_types.h"
#include "resource_id.h"

namespace crown
{

struct PhysicsResource
{
	uint32_t version;
	uint32_t num_controllers;		// 0 or 1, ATM
	uint32_t controller_offset;
	uint32_t num_actors;
	uint32_t actors_offset;
	uint32_t num_shapes_indices;
	uint32_t shapes_indices_offset;
	uint32_t num_shapes;
	uint32_t shapes_offset;
	uint32_t num_joints;
	uint32_t joints_offset;
};

struct PhysicsController
{
	StringId32 name;
	float height;				// Height of the capsule
	float radius;				// Radius of the capsule
	float slope_limit;			// The maximum slope which the character can walk up in radians.
	float step_offset;			// Maximum height of an obstacle which the character can climb.
	float contact_offset;		// Skin around the object within which contacts will be generated. Use it to avoid numerical precision issues.
	StringId32 collision_filter;// Collision filter from global.physics_config
};

struct PhysicsActor
{
	StringId32 name;			// Name of the actor
	StringId32 node;			// Node from .unit file
	StringId32 actor_class;		// Actor from global.physics
	float mass;					// Mass of the actor
	uint32_t num_shapes;		// Number of shapes
};

struct PhysicsShapeType
{
	enum Enum
	{
		SPHERE,
		CAPSULE,
		BOX,
		PLANE,
		CONVEX_MESH,

		COUNT
	};
};

struct PhysicsShape
{
	StringId32 name;			// Name of the shape
	StringId32 shape_class;		// Shape class from global.physics_config
	StringId32 type;			// Type of the shape
	StringId32 material;		// Material from global.physics_config
	ResourceId resource;		// Resource such as .mesh or .heightmap
	Vector3 position;			// In actor space
	Quaternion rotation;		// In actor space
	float data_0;
	float data_1;
	float data_2;
	float data_3;
};

struct PhysicsJointType
{
	enum Enum
	{
		FIXED,
		SPHERICAL,
		REVOLUTE,
		PRISMATIC,
		DISTANCE,
		D6,

		COUNT
	};
};

struct PhysicsJoint
{
	StringId32 name;
	uint32_t type;
	StringId32 actor_0;
	StringId32 actor_1;
	Vector3 anchor_0;
	Vector3 anchor_1;

	bool breakable;
	char _pad[3];
	float break_force;
	float break_torque;

	// Revolute/Prismatic Joint Limits
	float lower_limit;
	float upper_limit;

	// Spherical Joint Limits
	float y_limit_angle;
	float z_limit_angle;

	// Distance Joint Limits
	float max_distance;

	// JointLimitPair/cone param
	float contact_dist;

	float restitution;
	float spring;
	float damping;
	float distance;
};

namespace physics_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);

	bool has_controller(const PhysicsResource* pr);
	const PhysicsController* controller(const PhysicsResource* pr);
	uint32_t num_actors(const PhysicsResource* pr);
	const PhysicsActor* actor(const PhysicsResource* pr, uint32_t i);
	uint32_t num_shapes_indices(const PhysicsResource* pr);
	uint32_t shape_index(const PhysicsResource* pr, uint32_t i);
	uint32_t num_shapes(const PhysicsResource* pr);
	const PhysicsShape* shape(const PhysicsResource* pr, uint32_t i);
	uint32_t num_joints(const PhysicsResource* pr);
	const PhysicsJoint* joint(const PhysicsResource* pr, uint32_t i);
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

struct PhysicsMaterial
{
	float static_friction;
	float dynamic_friction;
	float restitution;
	// uint8_t restitution_combine_mode;
	// uint8_t friction_combine_mode;
};

struct PhysicsCollisionFilter
{
	uint32_t me;
	uint32_t mask;
};

struct PhysicsShape2
{
	StringId32 collision_filter;
	bool trigger;
	char _pad[3];
};

struct PhysicsActor2
{
	enum
	{
		DYNAMIC			= (1 << 0),
		KINEMATIC		= (1 << 1),
		DISABLE_GRAVITY	= (1 << 2)
	};

	float linear_damping;
	float angular_damping;
	uint32_t flags;
};

namespace physics_config_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);

	uint32_t num_materials(const PhysicsConfigResource* pcr);
	const PhysicsMaterial* material(const PhysicsConfigResource* pcr, StringId32 name);
	const PhysicsMaterial* material_by_index(const PhysicsConfigResource* pcr, uint32_t i);
	uint32_t num_shapes(const PhysicsConfigResource* pcr);
	const PhysicsShape2* shape(const PhysicsConfigResource* pcr, StringId32 name);
	const PhysicsShape2* shape_by_index(const PhysicsConfigResource* pcr, uint32_t i);
	uint32_t num_actors(const PhysicsConfigResource* pcr);
	const PhysicsActor2* actor(const PhysicsConfigResource* pcr, StringId32 name);
	const PhysicsActor2* actor_by_index(const PhysicsConfigResource* pcr, uint32_t i);
	uint32_t num_filters(const PhysicsConfigResource* pcr);
	const PhysicsCollisionFilter* filter(const PhysicsConfigResource* pcr, StringId32 name);
	const PhysicsCollisionFilter* filter_by_index(const PhysicsConfigResource* pcr, uint32_t i);
} // namespace physics_resource
} // namespace crown
