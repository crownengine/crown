/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/containers/vector.inl"
#   include "core/filesystem/filesystem.h"
#   include "core/json/json_object.inl"
#   include "core/json/sjson.h"
#   include "core/math/aabb.inl"
#   include "core/math/constants.h"
#   include "core/math/matrix4x4.inl"
#   include "core/math/vector2.inl"
#   include "core/math/vector3.inl"
#   include "core/memory/temp_allocator.inl"
#   include "core/strings/dynamic_string.inl"
#   include "resource/compile_options.inl"
#   include "resource/mesh.h"
#   include "resource/mesh_fbx.h"
#   include "resource/mesh_resource.h"
#   include <bx/error.h>
#   include <bx/readerwriter.h>
#   include <vertexlayout.h> // bgfx::write, bgfx::read

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
		array::clear(g._tangents);
		array::clear(g._bitangents);
		array::clear(g._bones);
		array::clear(g._weights);
		array::clear(g._uvs);

		array::clear(g._position_indices);
		array::clear(g._normal_indices);
		array::clear(g._tangent_indices);
		array::clear(g._bitangent_indices);
		array::clear(g._bone_indices);
		array::clear(g._weight_indices);
		array::clear(g._uv_indices);

		array::clear(g._vertex_buffer);
		array::clear(g._index_buffer);
	}

	bool has_normals(Geometry &g)
	{
		return array::size(g._normals) != 0;
	}

	bool has_tangents(Geometry &g)
	{
		return array::size(g._tangents) != 0;
	}

	bool has_bitangents(Geometry &g)
	{
		return array::size(g._bitangents) != 0;
	}

	bool has_bones(Geometry &g)
	{
		return array::size(g._bones) != 0;
	}

	bool has_uvs(Geometry &g)
	{
		return array::size(g._uvs) != 0;
	}

	static u32 vertex_stride(Geometry &g)
	{
		u32 stride = 0;
		stride += 3 * sizeof(f32);
		stride += has_normals(g) ? 3*sizeof(f32) : 0;
		stride += has_tangents(g) ? 3*sizeof(f32) : 0;
		stride += has_bitangents(g) ? 3*sizeof(f32) : 0;
		stride += has_bones(g) ? 8*sizeof(f32) : 0;
		stride += has_uvs(g) ? 2*sizeof(f32) : 0;
		return stride;
	}

	static bgfx::VertexLayout vertex_layout(Geometry &g)
	{
		bgfx::VertexLayout layout;
		memset((void *)&layout, 0, sizeof(layout));

		layout.begin();
		layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

		if (has_normals(g))
			layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true);

		if (has_tangents(g))
			layout.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float, true);

		if (has_bitangents(g))
			layout.add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float, true);

		if (has_bones(g)) {
			layout.add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Float);
			layout.add(bgfx::Attrib::Weight, 4, bgfx::AttribType::Float);
		}

		if (has_uvs(g))
			layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);

		layout.end();
		return layout;
	}

	static void generate_vertex_and_index_buffers(Geometry &g)
	{
		array::resize(g._index_buffer, array::size(g._position_indices));

		u16 index = 0;
		for (u32 i = 0; i < array::size(g._position_indices); ++i) {
			g._index_buffer[i] = index++;

			const u16 idx = g._position_indices[i] * 3;
			Vector3 v;
			v.x = g._positions[idx + 0];
			v.y = g._positions[idx + 1];
			v.z = g._positions[idx + 2];
			array::push(g._vertex_buffer, (char *)&v, sizeof(v));

			if (has_normals(g)) {
				const u16 idx = g._normal_indices[i] * 3;
				Vector3 v;
				v.x = g._normals[idx + 0];
				v.y = g._normals[idx + 1];
				v.z = g._normals[idx + 2];
				array::push(g._vertex_buffer, (char *)&v, sizeof(v));
			}

			if (has_tangents(g)) {
				const u16 idx = g._tangent_indices[i] * 3;
				Vector3 v;
				CE_ENSURE(idx < array::size(g._tangents));
				v.x = g._tangents[idx + 0];
				v.y = g._tangents[idx + 1];
				v.z = g._tangents[idx + 2];
				array::push(g._vertex_buffer, (char *)&v, sizeof(v));
			}

			if (has_bitangents(g)) {
				const u16 idx = g._bitangent_indices[i] * 3;
				CE_ENSURE(idx < array::size(g._bitangents));
				Vector3 v;
				v.x = g._bitangents[idx + 0];
				v.y = g._bitangents[idx + 1];
				v.z = g._bitangents[idx + 2];
				array::push(g._vertex_buffer, (char *)&v, sizeof(v));
			}

			if (has_bones(g)) {
				const u16 bidx = g._bone_indices[i] * 4;
				Vector4 b;
				b.x = g._bones[bidx + 0];
				b.y = g._bones[bidx + 1];
				b.z = g._bones[bidx + 2];
				b.w = g._bones[bidx + 3];
				array::push(g._vertex_buffer, (char *)&b, sizeof(b));

				const u16 widx = g._weight_indices[i] * 4;
				Vector4 w;
				w.x = g._weights[widx + 0];
				w.y = g._weights[widx + 1];
				w.z = g._weights[widx + 2];
				w.w = g._weights[widx + 3];
				array::push(g._vertex_buffer, (char *)&w, sizeof(w));
			}

			if (has_uvs(g)) {
				const u16 idx = g._uv_indices[i] * 2;
				CE_ENSURE(idx < array::size(g._uvs));
				Vector2 v;
				v.x = g._uvs[idx + 0];
				v.y = g._uvs[idx + 1];
				array::push(g._vertex_buffer, (char *)&v, sizeof(v));
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

		if (array::size(g._positions) != 0) {
			aabb::from_points(aabb
				, array::size(g._positions) / 3
				, sizeof(g._positions[0]) * 3
				, array::begin(g._positions)
				);
		}

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

	static s32 parse_internal(Mesh &m, Buffer &buf, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		DynamicString source(ta);
		if (json_object::has(obj, "source")) {
			RETURN_IF_ERROR(sjson::parse_string(source, obj["source"]), opts);

			RETURN_IF_FILE_MISSING(source.c_str(), opts);
			Buffer fbx_buf = opts.read(source.c_str());
			return fbx::parse(m, fbx_buf, opts);
		} else {
			return mesh::parse(m, buf, opts);
		}
	}

	s32 parse(Mesh &m, const char *path, CompileOptions &opts)
	{
		RETURN_IF_FILE_MISSING(path, opts);
		Buffer buf = opts.read(path);
		return parse_internal(m, buf, opts);
	}

	s32 parse(Mesh &m, CompileOptions &opts)
	{
		Buffer buf = opts.read();
		return parse_internal(m, buf, opts);
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
	, _bitangents(a)
	, _bones(a)
	, _weights(a)
	, _position_indices(a)
	, _normal_indices(a)
	, _tangent_indices(a)
	, _bitangent_indices(a)
	, _bone_indices(a)
	, _weight_indices(a)
	, _uv_indices(a)
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

#endif // if CROWN_CAN_COMPILE
