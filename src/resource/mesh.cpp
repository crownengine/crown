/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "resource/compile_options.inl"
#include "resource/mesh.h"
#include "resource/mesh_resource.h"
#include <bx/error.h>
#include <bx/readerwriter.h>
#include <vertexlayout.h> // bgfx::write, bgfx::read

namespace crown
{
namespace mesh
{
	/// Writer interface.
	struct BgfxWriter : public bx::WriterI
	{
		BinaryWriter *_bw;

		///
		explicit BgfxWriter(BinaryWriter &bw)
			: _bw(&bw)
		{
		}

		///
		virtual ~BgfxWriter()
		{
		}

		///
		virtual int32_t write(const void *_data, int32_t _size, bx::Error *_err)
		{
			CE_UNUSED(_err);
			_bw->write(_data, _size);
			return _size; // FIXME: return the actual number of bytes written
		}
	};

	static void reset(Geometry &g)
	{
		array::clear(g._positions);
		array::clear(g._normals);
		array::clear(g._uvs);
		array::clear(g._tangents);
		array::clear(g._binormals);

		array::clear(g._position_indices);
		array::clear(g._normal_indices);
		array::clear(g._uv_indices);
		array::clear(g._tangent_indices);
		array::clear(g._binormal_indices);

		array::clear(g._vertex_buffer);
		array::clear(g._index_buffer);
	}

	bool has_normals(Geometry &g)
	{
		return array::size(g._normals) != 0;
	}

	bool has_uvs(Geometry &g)
	{
		return array::size(g._uvs) != 0;
	}

	static u32 vertex_stride(Geometry &g)
	{
		u32 stride = 0;
		stride += 3 * sizeof(f32);
		stride += (has_normals(g) ? 3 * sizeof(f32) : 0);
		stride += (has_uvs(g)     ? 2 * sizeof(f32) : 0);
		return stride;
	}

	static bgfx::VertexLayout vertex_layout(Geometry &g)
	{
		bgfx::VertexLayout layout;
		memset((void *)&layout, 0, sizeof(layout));

		layout.begin();
		layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

		if (has_normals(g)) {
			layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true);
		}
		if (has_uvs(g)) {
			layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
		}

		layout.end();
		return layout;
	}

	static void generate_vertex_and_index_buffers(Geometry &g)
	{
		array::resize(g._index_buffer, array::size(g._position_indices));

		u16 index = 0;
		for (u32 i = 0; i < array::size(g._position_indices); ++i) {
			g._index_buffer[i] = index++;

			const u16 p_idx = g._position_indices[i] * 3;
			Vector3 xyz;
			xyz.x = g._positions[p_idx + 0];
			xyz.y = g._positions[p_idx + 1];
			xyz.z = g._positions[p_idx + 2];
			array::push(g._vertex_buffer, (char *)&xyz, sizeof(xyz));

			if (has_normals(g)) {
				const u16 n_idx = g._normal_indices[i] * 3;
				Vector3 n;
				n.x = g._normals[n_idx + 0];
				n.y = g._normals[n_idx + 1];
				n.z = g._normals[n_idx + 2];
				array::push(g._vertex_buffer, (char *)&n, sizeof(n));
			}
			if (has_uvs(g)) {
				const u16 t_idx = g._uv_indices[i] * 2;
				Vector2 uv;
				uv.x = g._uvs[t_idx + 0];
				uv.y = g._uvs[t_idx + 1];
				array::push(g._vertex_buffer, (char *)&uv, sizeof(uv));
			}
		}
	}

	static void geometry_names(Vector<DynamicString> &names, const Mesh &m, const DynamicString &geometry)
	{
		auto cur = hash_map::begin(m._nodes);
		auto end = hash_map::end(m._nodes);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(m._nodes, cur);

			if (cur->second._geometry == geometry)
				vector::push_back(names, cur->first);
		}
	}

	static OBB obb(Geometry &g)
	{
		AABB aabb;
		OBB obb;
		aabb::reset(aabb);
		memset(&obb, 0, sizeof(obb));

		aabb::from_points(aabb
			, array::size(g._positions) / 3
			, sizeof(g._positions[0]) * 3
			, array::begin(g._positions)
			);

		obb.tm = from_quaternion_translation(QUATERNION_IDENTITY, aabb::center(aabb));
		obb.half_extents = (aabb.max - aabb.min) * 0.5f;
		return obb;
	}

	s32 write(Mesh &m, CompileOptions &opts)
	{
		opts.write(RESOURCE_HEADER(RESOURCE_VERSION_MESH));
		opts.write(hash_map::size(m._geometries));

		auto cur = hash_map::begin(m._geometries);
		auto end = hash_map::end(m._geometries);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(m._geometries, cur);

			Vector<DynamicString> geo_names(default_allocator());
			geometry_names(geo_names, m, cur->first);
			u32 num_geo_names = vector::size(geo_names);

			opts.write(num_geo_names);
			for (u32 i = 0; i < num_geo_names; ++i)
				opts.write(geo_names[i].to_string_id()._id);

			Geometry *geo = (Geometry *)&cur->second;
			mesh::generate_vertex_and_index_buffers(*geo);

			bgfx::VertexLayout layout = mesh::vertex_layout(*geo);
			u32 stride = mesh::vertex_stride(*geo);
			OBB bbox = mesh::obb(*geo);

			BgfxWriter writer(opts._binary_writer);
			bgfx::write(&writer, layout);
			opts.write(bbox);

			opts.write(array::size(geo->_vertex_buffer) / stride);
			opts.write(stride);
			opts.write(array::size(geo->_index_buffer));

			opts.write(geo->_vertex_buffer);
			opts.write(array::begin(geo->_index_buffer), array::size(geo->_index_buffer) * sizeof(u16));
		}

		return 0;
	}

	s32 parse(Mesh &m, CompileOptions &opts, const char *path)
	{
		if (str_has_suffix(path, ".mesh")) {
			Buffer buf = opts.read(path);
			return mesh::parse(m, buf, opts);
		} else {
			TempAllocator512 ta;
			DynamicString str(ta);
			str = path;
			str += ".mesh";
			Buffer buf = opts.read(str.c_str());
			return mesh::parse(m, buf, opts);
		}
	}

} // namespace mesh

Node::Node(Allocator &a)
	: _local_pose(MATRIX4X4_IDENTITY)
	, _geometry(a)
{
}

Geometry::Geometry(Allocator &a)
	: _positions(a)
	, _normals(a)
	, _uvs(a)
	, _tangents(a)
	, _binormals(a)
	, _position_indices(a)
	, _normal_indices(a)
	, _uv_indices(a)
	, _tangent_indices(a)
	, _binormal_indices(a)
	, _vertex_buffer(a)
	, _index_buffer(a)
{
	mesh::reset(*this);
}

Mesh::Mesh(Allocator &a)
	: _geometries(a)
	, _nodes(a)
{
}

} // namespace crown
