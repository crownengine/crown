/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/filesystem/types.h"
#   include "core/list.h"
#   include "core/math/types.h"
#   include "core/memory/types.h"
#   include "core/strings/dynamic_string.h"
#   include "resource/types.h"

namespace crown
{
struct Node
{
	ALLOCATOR_AWARE;

	Matrix4x4 _local_pose;
	DynamicString _geometry;

	///
	explicit Node(Allocator &a);
};

struct Geometry
{
	ALLOCATOR_AWARE;

	enum { MAX_BONE_WEIGHTS = 4 };

	Array<f32> _positions;
	Array<f32> _normals;
	Array<f32> _uvs;
	Array<f32> _tangents;
	Array<f32> _bitangents;
	Array<f32> _bones;
	Array<f32> _weights;

	Array<u32> _position_indices;
	Array<u32> _normal_indices;
	Array<u32> _tangent_indices;
	Array<u32> _bitangent_indices;
	Array<u32> _bone_indices;
	Array<u32> _weight_indices;
	Array<u32> _uv_indices;

	Array<char> _vertex_buffer;
	Array<u16> _index_buffer;

	///
	explicit Geometry(Allocator &a);
};

struct Mesh
{
	ListNode _cache_node;
	StringId64 _path;
	HashMap<DynamicString, Geometry> _geometries;
	HashMap<DynamicString, Node> _nodes;

	///
	explicit Mesh(Allocator &a);
};

namespace mesh
{
	///
	bool has_normals(Geometry &g);

	///
	bool has_tangents(Geometry &g);

	///
	bool has_bitangents(Geometry &g);

	///
	bool has_bones(Geometry &g);

	///
	bool has_uvs(Geometry &g);

	///
	s32 parse(Mesh &m, const char *path, CompileOptions &opts);

	///
	s32 parse(Mesh &m, CompileOptions &opts);

	///
	s32 write(Mesh &m, CompileOptions &opts);

} // namespace mesh

struct MeshCache
{
	ListNode _meshes;

	///
	MeshCache();

	///
	~MeshCache();
};

namespace mesh_cache
{
	///
	Mesh *get(MeshCache &cache, const char *path);

	///
	void add(MeshCache &cache, Mesh *mesh);

} // namespace mesh_cache

} // namespace crown

#endif // if CROWN_CAN_COMPILE
