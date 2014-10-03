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

#include "types.h"
#include "resource.h"
#include "bundle.h"
#include "allocator.h"
#include "file.h"
#include "physics_types.h"
#include "matrix4x4.h"
#include "camera.h"

namespace crown
{

// All offsets are absolute
struct UnitHeader
{
	ResourceId physics_resource;
	StringId64 sprite_animation;
	uint32_t num_renderables;
	uint32_t renderables_offset;
	uint32_t num_materials;
	uint32_t materials_offset;
	uint32_t num_cameras;
	uint32_t cameras_offset;
	uint32_t num_scene_graph_nodes;
	uint32_t scene_graph_nodes_offset;
	uint32_t num_keys;
	uint32_t keys_offset;
	uint32_t values_size;
	uint32_t values_offset;
};

struct UnitRenderable
{
	enum { MESH, SPRITE } type;
	ResourceId resource;
	StringId32 name;
	int32_t node;
	bool visible;
};

struct UnitMaterial
{
	StringId64 id;
};

struct UnitCamera
{
	uint32_t name;
	int32_t node;

	ProjectionType::Enum type;
	float fov;
	float near;
	float far;
};

struct UnitNode
{
	StringId32 name;
	Matrix4x4 pose;
	int32_t parent;
};

struct ValueType
{
	enum Enum
	{
		BOOL,
		FLOAT,
		STRING,
		VECTOR3
	};
};

struct Key
{
	StringId32 name;
	uint32_t type;
	uint32_t offset;
};

struct UnitResource
{
	ResourceId sprite_animation() const
	{
		ResourceId id;
		id.type = SPRITE_ANIMATION_TYPE;
		id.name = ((UnitHeader*) this)->sprite_animation;
		return id;
	}

	//-----------------------------------------------------------------------------
	ResourceId physics_resource() const
	{
		return ((UnitHeader*) this)->physics_resource;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_renderables() const
	{
		return ((UnitHeader*) this)->num_renderables;
	}

	//-----------------------------------------------------------------------------
	UnitRenderable get_renderable(uint32_t i) const
	{
		CE_ASSERT(i < num_renderables(), "Index out of bounds");

		UnitHeader* h = (UnitHeader*) this;
		UnitRenderable* begin = (UnitRenderable*) (((char*) this) + h->renderables_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_materials() const
	{
		return ((UnitHeader*) this)->num_materials;
	}

	//-----------------------------------------------------------------------------
	UnitMaterial get_material(uint32_t i) const
	{
		CE_ASSERT(i < num_materials(), "Index out of bounds");

		UnitHeader* h = (UnitHeader*) this;
		UnitMaterial* begin = (UnitMaterial*) (((char*) this) + h->materials_offset);
		return begin[i];
	}	

	//-----------------------------------------------------------------------------
	uint32_t num_cameras() const
	{
		return ((UnitHeader*) this)->num_cameras;
	}

	//-----------------------------------------------------------------------------
	UnitCamera get_camera(uint32_t i) const
	{
		CE_ASSERT(i < num_cameras(), "Index out of bounds");

		UnitHeader* h = (UnitHeader*) this;
		UnitCamera* begin = (UnitCamera*) (((char*) this) + h->cameras_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_scene_graph_nodes() const
	{
		return ((UnitHeader*) this)->num_scene_graph_nodes;
	}

	//-----------------------------------------------------------------------------
	UnitNode* scene_graph_nodes() const
	{
		UnitHeader* h = (UnitHeader*) this;
		return (UnitNode*) (((char*) this) + h->scene_graph_nodes_offset);
	}

	//-----------------------------------------------------------------------------
	uint32_t num_keys() const
	{
		return ((UnitHeader*) this)->num_keys;
	}

	//-----------------------------------------------------------------------------
	bool has_key(const char* k) const
	{
		UnitHeader* h = (UnitHeader*) this;
		const uint32_t nk = num_keys();
		Key* begin = (Key*) (((char*) this) + h->keys_offset);

		for (uint32_t i = 0; i < nk; i++)
		{
			if (begin[i].name == string::murmur2_32(k, string::strlen(k)))
			{
				return true;
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	bool get_key(const char* k, Key& out_k) const
	{
		UnitHeader* h = (UnitHeader*) this;
		const uint32_t nk = num_keys();
		Key* begin = (Key*) (((char*) this) + h->keys_offset);

		for (uint32_t i = 0; i < nk; i++)
		{
			if (begin[i].name == string::murmur2_32(k, string::strlen(k)))
			{
				out_k = begin[i];
				return true;
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	uint32_t values_size() const
	{
		return ((UnitHeader*) this)->values_size;
	}

	//-----------------------------------------------------------------------------
	const char* values() const
	{
		UnitHeader* h = (UnitHeader*) this;
		return ((char*) this) + h->values_offset;
	}

private:

	// Disable construction
	UnitResource();
};

namespace unit_resource
{
	void compile(Filesystem& fs, const char* resource_path, File* out_file);
	inline void compile(const char* path, CompileOptions& opts)
	{
		compile(opts._fs, path, &opts._bw.m_file);
	}
	void* load(Allocator& allocator, Bundle& bundle, ResourceId id);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);
} // namespace unit_resource
} // namespace crown
