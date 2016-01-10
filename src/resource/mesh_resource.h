/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"
#include "math_types.h"
#include "container_types.h"
#include "array.h"
#include <bgfx/bgfx.h>

namespace crown
{

struct VertexData
{
	uint32_t num;
	uint32_t stride;
	char* data;
};

struct IndexData
{
	uint32_t num;
	char* data;   // size = num*sizeof(uint16_t)
};

struct MeshGeometry
{
	bgfx::VertexDecl decl;
	bgfx::VertexBufferHandle vertex_buffer;
	bgfx::IndexBufferHandle index_buffer;
	OBB obb;
	VertexData vertices;
	IndexData indices;
};

struct MeshResource
{
	Array<StringId32> geometry_names;
	Array<MeshGeometry*> geometries;

	MeshResource(Allocator& a)
		: geometry_names(a)
		, geometries(a)
	{
	}

	const MeshGeometry* geometry(StringId32 name) const
	{
		for (uint32_t i = 0; i < array::size(geometry_names); ++i)
		{
			if (geometry_names[i] == name)
				return geometries[i];
		}

		CE_ASSERT(false, "Mesh name not found");
		return NULL;
	}
};

namespace mesh_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& a, void* res);
}
} // namespace crown
