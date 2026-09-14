/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/strings/string_id.h"

namespace crown
{
struct MeshMaterialRange
{
	// Compiled geometries contain one contiguous range per material slot.
	StringId32 slot;
	u32 index_offset;
	u32 num_indices;
};

} // namespace crown

#if CROWN_CAN_COMPILE
#   include "core/filesystem/types.h"
#   include "core/list.h"
#   include "core/math/types.h"
#   include "core/memory/types.h"
#   include "core/strings/dynamic_string.h"
#   include "core/thread/mutex.h"
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
	Array<MeshMaterialRange> _material_ranges;

	///
	explicit Geometry(Allocator &a);
};

struct DataRange
{
	u32 offset;
	u32 count;
};

struct GeometryInfo
{
	DataRange _positions;
	DataRange _normals;
	DataRange _uvs;
	DataRange _tangents;
	DataRange _bitangents;
	DataRange _bones;
	DataRange _weights;

	DataRange _position_indices;
	DataRange _normal_indices;
	DataRange _tangent_indices;
	DataRange _bitangent_indices;
	DataRange _bone_indices;
	DataRange _weight_indices;
	DataRange _uv_indices;

	DataRange _material_ranges;
};

struct Mesh
{
	ListNode _cache_node;
	StringId64 _path;
	Geometry _geometry;
	HashMap<DynamicString, GeometryInfo> _geometries;
	HashMap<DynamicString, Node> _nodes;

	///
	explicit Mesh(Allocator &a);
};

namespace mesh
{
	///
	bool has_normals(const GeometryInfo &g);

	///
	bool has_tangents(const GeometryInfo &g);

	///
	bool has_bitangents(const GeometryInfo &g);

	///
	bool has_bones(const GeometryInfo &g);

	///
	bool has_uvs(const GeometryInfo &g);

	///
	s32 parse(const Mesh **m, const char *path, CompileOptions &opts);

	///
	s32 parse(Mesh &m, CompileOptions &opts);

	///
	s32 write(Mesh &m, CompileOptions &opts);

} // namespace mesh

struct MeshCache
{
	ListNode _meshes;
	Mutex _mutex;

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
	Mesh *add(MeshCache &cache, Mesh *mesh);

	///
	void clear(MeshCache &cache);

} // namespace mesh_cache

} // namespace crown

#endif // if CROWN_CAN_COMPILE
