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

#include "Types.h"
#include "Allocator.h"
#include "File.h"
#include "Bundle.h"
#include "ResourceManager.h"

namespace crown
{

struct PhysicsHeader
{
	uint32_t version;
	uint32_t num_controllers;		// 0 or 1, ATM
	uint32_t controller_offset;
	uint32_t num_actors;
	uint32_t actors_offset;
};

struct PhysicsController
{
	StringId32 name;
	float height;			// Height of the capsule
	float radius;			// Radius of the capsule
	float slope_limit;		// The maximum slope which the character can walk up in radians.
	float step_offset;		// Maximum height of an obstacle which the character can climb.
	float contact_offset;	// Skin around the object within which contacts will be generated. Use it to avoid numerical precision issues.
};

struct PhysicsActor
{
	StringId32 name;
	StringId32 node;
	uint32_t num_shapes;
};

struct PhysicsShapeType
{
	enum Enum
	{
		SPHERE,
		CAPSULE,
		BOX,
		PLANE
	};
};

struct PhysicsShape
{
	StringId32 name;
	uint32_t type;
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

private:

	// Disable construction
	PhysicsResource();
};

} // namespace crown