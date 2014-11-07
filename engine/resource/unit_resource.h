/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy ofur software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice andur permission notice shall be
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

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "compiler_types.h"

namespace crown
{

// All offsets are absolute
struct UnitResource
{
	uint32_t version;
	uint32_t _pad;
	StringId64 physics_resource;
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
	enum Enum { MESH, SPRITE };
	uint32_t type;
	uint32_t _pad;
	StringId64 resource;
	StringId32 name;
	int32_t node;
	bool visible;
	char _pad1[3];
	char _pad2[4];
};

struct UnitMaterial
{
	StringId64 id;
};

struct UnitCamera
{
	uint32_t name;
	int32_t node;
	uint32_t type; // ProjectionType::Enum
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

namespace unit_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);

	StringId64 sprite_animation(const UnitResource* ur);
	StringId64 physics_resource(const UnitResource* ur);
	uint32_t num_renderables(const UnitResource* ur);
	const UnitRenderable* get_renderable(const UnitResource* ur, uint32_t i);
	uint32_t num_materials(const UnitResource* ur);
	const UnitMaterial* get_material(const UnitResource* ur, uint32_t i);
	uint32_t num_cameras(const UnitResource* ur);
	const UnitCamera* get_camera(const UnitResource* ur, uint32_t i);
	uint32_t num_scene_graph_nodes(const UnitResource* ur);
	const UnitNode* scene_graph_nodes(const UnitResource* ur);
	uint32_t num_keys(const UnitResource* ur);
	bool has_key(const UnitResource* ur, const char* k);
	bool get_key(const UnitResource* ur, const char* k, Key& out_k);
	uint32_t values_size(const UnitResource* ur);
	const char* values(const UnitResource* ur);
} // namespace unit_resource
} // namespace crown
