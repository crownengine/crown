/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <algorithm>
#include "Types.h"
#include "Allocator.h"
#include "File.h"
#include "Bundle.h"
#include "ResourceManager.h"
#include "Vector3.h"

namespace crown
{

//-----------------------------------------------------------------------------
struct PhysicsHeader
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
struct PhysicsActor
{
	StringId32 name;			// Name of the actor
	StringId32 node;			// Node from .unit file
	StringId32 actor_class;		// Actor from global.physics
	uint32_t num_shapes;		// Number of shapes
};

//-----------------------------------------------------------------------------
struct PhysicsShapeType
{
	enum Enum
	{
		SPHERE,
		CAPSULE,
		BOX,
		PLANE,
		CONVEX_MESH
	};
};

//-----------------------------------------------------------------------------
struct PhysicsShape
{
	StringId32 name;			// Name of the shape
	StringId32 shape_class;		// Shape class from global.physics_config
	StringId32 type;			// Type of the shape
	StringId32 material;		// Material from global.physics_config
	StringId32 resource;		// Resource such as .mesh or .heightmap
	float data_0;
	float data_1;
	float data_2;
	float data_3;
};

//-----------------------------------------------------------------------------
struct PhysicsJointType
{
	enum Enum
	{
		FIXED,
		SPHERICAL,
		REVOLUTE,
		PRISMATIC,
		DISTANCE,
		D6
	};
};

//-----------------------------------------------------------------------------
struct PhysicsJoint
{
	StringId32 name;
	uint32_t type;
	StringId32 actor_0;
	StringId32 actor_1;
	Vector3 anchor_0;
	Vector3 anchor_1;

	bool breakable;
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

//-----------------------------------------------------------------------------
struct PhysicsResource
{
	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
	}

	//-----------------------------------------------------------------------------
	bool has_controller() const
	{
		return ((PhysicsHeader*) this)->num_controllers == 1;
	}

	//-----------------------------------------------------------------------------
	PhysicsController controller() const
	{
		CE_ASSERT(has_controller(), "Controller does not exist");
		const PhysicsHeader* ph = (PhysicsHeader*) this;
		PhysicsController* controller = (PhysicsController*) (((char*) this) + ph->controller_offset);
		return *controller;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_actors() const
	{
		return ((PhysicsHeader*) this)->num_actors;
	}

	//-----------------------------------------------------------------------------
	PhysicsActor actor(uint32_t i) const
	{
		CE_ASSERT(i < num_actors(), "Index out of bounds");
		const PhysicsHeader* ph = (PhysicsHeader*) this;
		PhysicsActor* actor = (PhysicsActor*) (((char*) this) + ph->actors_offset);
		return actor[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_shapes_indices() const
	{
		return ((PhysicsHeader*) this)->num_shapes_indices;
	}

	//-----------------------------------------------------------------------------
	uint32_t shape_index(uint32_t i) const
	{
		CE_ASSERT(i < num_shapes_indices(), "Index out of bounds");

		const PhysicsHeader* ph = (PhysicsHeader*) this;
		uint32_t* index = (uint32_t*) (((char*) this) + ph->shapes_indices_offset);
		return index[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_shapes() const
	{
		return ((PhysicsHeader*) this)->num_shapes;
	}

	//-----------------------------------------------------------------------------
	PhysicsShape shape(uint32_t i) const
	{
		CE_ASSERT(i < num_shapes(), "Index out of bounds");

		const PhysicsHeader* ph = (PhysicsHeader*) this;
		PhysicsShape* shape = (PhysicsShape*) (((char*) this) + ph->shapes_offset);
		return shape[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_joints() const
	{
		return ((PhysicsHeader*) this)->num_joints;
	}

	//-----------------------------------------------------------------------------
	PhysicsJoint joint(uint32_t i) const
	{
		CE_ASSERT(i < num_joints(), "Index out of bounds");

		const PhysicsHeader* ph = (PhysicsHeader*) this;
		PhysicsJoint* joint = (PhysicsJoint*) (((char*) this) + ph->joints_offset);
		return joint[i];
	}

private:

	// Disable construction
	PhysicsResource();
};

struct PhysicsConfigHeader
{
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
	uint32_t collision_filter;
	bool trigger;
};

struct PhysicsActor2
{
	enum
	{
		DYNAMIC			= (1 << 0),
		KINEMATIC		= (1 << 1),
		DISABLE_GRAVITY	= (1 << 2)
	};

	uint32_t collision_filter;
	float linear_damping;
	float angular_damping;
	uint8_t flags;
};

//-----------------------------------------------------------------------------
struct PhysicsConfigResource
{
	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
	}

	//-----------------------------------------------------------------------------
	uint32_t num_materials() const
	{
		return ((PhysicsConfigHeader*) this)->num_materials;
	}

	/// Returns the material with the given @a name
	PhysicsMaterial material(StringId32 name) const
	{
		const PhysicsConfigHeader* h = (PhysicsConfigHeader*) this;
		StringId32* begin = (StringId32*) (((char*) this) + h->materials_offset);
		StringId32* end = begin + num_materials();
		StringId32* id = std::find(begin, end, name);
		CE_ASSERT(id != end, "Material not found");
		return material_by_index(id - begin);
	}

	PhysicsMaterial material_by_index(uint32_t i) const
	{
		CE_ASSERT(i < num_materials(), "Index out of bounds");
		const PhysicsConfigHeader* h = (PhysicsConfigHeader*) this;
		const PhysicsMaterial* base = (PhysicsMaterial*) (((char*) this) + h->materials_offset + sizeof(StringId32) * num_materials());
		return base[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_shapes() const
	{
		return ((PhysicsConfigHeader*) this)->num_shapes;
	}

	//-----------------------------------------------------------------------------
	PhysicsShape2 shape(StringId32 name) const
	{
		const PhysicsConfigHeader* h = (PhysicsConfigHeader*) this;
		StringId32* begin = (StringId32*) (((char*) this) + h->shapes_offset);
		StringId32* end = begin + num_shapes();
		StringId32* id = std::find(begin, end, name);
		CE_ASSERT(id != end, "Shape not found");
		return shape_by_index(id - begin);
	}

	//-----------------------------------------------------------------------------
	PhysicsShape2 shape_by_index(uint32_t i) const
	{
		CE_ASSERT(i < num_shapes(), "Index out of bounds");
		const PhysicsConfigHeader* h = (PhysicsConfigHeader*) this;
		const PhysicsShape2* base = (PhysicsShape2*) (((char*) this) + h->shapes_offset + sizeof(StringId32) * num_shapes());
		return base[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_actors() const
	{
		return ((PhysicsConfigHeader*) this)->num_actors;
	}

	/// Returns the actor with the given @a name
	PhysicsActor2 actor(StringId32 name) const
	{
		const PhysicsConfigHeader* h = (PhysicsConfigHeader*) this;
		StringId32* begin = (StringId32*) (((char*) this) + h->actors_offset);
		StringId32* end = begin + num_actors();
		StringId32* id = std::find(begin, end, name);
		CE_ASSERT(id != end, "Actor not found");
		return actor_by_index(id - begin);
	}

	//-----------------------------------------------------------------------------
	PhysicsActor2 actor_by_index(uint32_t i) const
	{
		CE_ASSERT(i < num_actors(), "Index out of bounds");
		const PhysicsConfigHeader* h = (PhysicsConfigHeader*) this;
		const PhysicsActor2* base = (PhysicsActor2*) (((char*) this) + h->actors_offset + sizeof(StringId32) * num_actors());
		return base[i];
	}

private:

	// Disable construction
	PhysicsConfigResource();
};

} // namespace crown