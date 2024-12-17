/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/dynamic_string.h"
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

struct MeshResource
{
	Array<StringId32> geometry_names;
	Array<MeshGeometry *> geometries;

	///
	explicit MeshResource(Allocator &a);

	///
	const MeshGeometry *geometry(StringId32 name) const;
};

namespace mesh_resource_internal
{
	struct Node
	{
		ALLOCATOR_AWARE;

		Matrix4x4 _local_pose;

		///
		explicit Node(Allocator &a);
	};

	struct Geometry
	{
		ALLOCATOR_AWARE;

		Array<f32> _positions;
		Array<f32> _normals;
		Array<f32> _uvs;
		Array<f32> _tangents;
		Array<f32> _binormals;

		Array<u16> _position_indices;
		Array<u16> _normal_indices;
		Array<u16> _uv_indices;
		Array<u16> _tangent_indices;
		Array<u16> _binormal_indices;

		Array<char> _vertex_buffer;
		Array<u16> _index_buffer;

		///
		explicit Geometry(Allocator &a);
	};

	struct Mesh
	{
		HashMap<DynamicString, Geometry> _geometries;
		HashMap<DynamicString, Node> _nodes;

		///
		explicit Mesh(Allocator &a);
	};

	namespace mesh
	{
		///
		s32 parse(Mesh &m, CompileOptions &opts, const char *path);

	} // namespace mesh

	s32 compile(CompileOptions &opts);
	void *load(File &file, Allocator &a);
	void online(StringId64 /*id*/, ResourceManager & /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager & /*rm*/);
	void unload(Allocator &a, void *res);

} // namespace mesh_resource_internal

} // namespace crown
