/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "array.h"
#include "compiler_types.h"
#include "container_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct VertexData
{
	u32 num;
	u32 stride;
	char* data;
};

struct IndexData
{
	u32 num;
	char* data; // size = num*sizeof(u16)
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
		for (u32 i = 0; i < array::size(geometry_names); ++i)
		{
			if (geometry_names[i] == name)
				return geometries[i];
		}

		CE_FATAL("Mesh name not found");
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
