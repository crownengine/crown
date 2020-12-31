/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"
#include "resource/types.h"
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
	bgfx::VertexLayout layout;
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

	///
	MeshResource(Allocator& a);

	///
	const MeshGeometry* geometry(StringId32 name) const;
};

namespace mesh_resource_internal
{
	s32 compile(CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& a, void* res);

} // namespace mesh_resource_internal

} // namespace crown
