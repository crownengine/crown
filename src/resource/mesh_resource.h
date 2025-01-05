/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_id.h"
#include "resource/mesh.h"
#include "resource/types.h"
#include <bgfx/bgfx.h>

namespace crown
{
struct VertexData
{
	u32 num;
	u32 stride;
	char *data;
};

struct IndexData
{
	u32 num;
	char *data; // size = num*sizeof(u16)
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

struct MeshNode
{
	StringId32 name;
	u32 geometry_index;
};

struct MeshResource
{
	Array<MeshNode> nodes;
	Array<MeshGeometry *> geometries;

	///
	explicit MeshResource(Allocator &a);

	///
	const MeshGeometry *geometry(StringId32 name) const;
};

#if CROWN_CAN_COMPILE
namespace mesh
{
	///
	s32 parse(Mesh &m, Buffer &buf, CompileOptions &opts);

} // namespace mesh

namespace mesh_resource_internal
{
	///
	s32 compile(CompileOptions &opts);

} // namespace mesh_resource_internal
#endif

namespace mesh_resource_internal
{
	///
	void *load(File &file, Allocator &a);

	///
	void online(StringId64 /*id*/, ResourceManager & /*rm*/);

	///
	void offline(StringId64 /*id*/, ResourceManager & /*rm*/);

	///
	void unload(Allocator &a, void *res);

} // namespace mesh_resource_internal

} // namespace crown
