/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/reader_writer.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/mesh_resource.h"
#include "resource/resource_manager.h"
#include <bx/readerwriter.h>
#include <bx/error.h>
#include <vertexlayout.h> // bgfx::write, bgfx::read

namespace crown
{
struct BgfxReader : public bx::ReaderI
{
	BinaryReader *_br;

	///
	explicit BgfxReader(BinaryReader &br)
		: _br(&br)
	{
	}

	///
	virtual ~BgfxReader()
	{
	}

	///
	virtual int32_t read(void *_data, int32_t _size, bx::Error *_err)
	{
		CE_UNUSED(_err);
		_br->read(_data, _size);
		return _size; // FIXME: return the actual number of bytes read
	}
};

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

MeshResource::MeshResource(Allocator &a)
	: geometry_names(a)
	, geometries(a)
{
}

const MeshGeometry *MeshResource::geometry(StringId32 name) const
{
	for (u32 i = 0; i < array::size(geometry_names); ++i) {
		if (geometry_names[i] == name)
			return geometries[i];
	}

	CE_FATAL("Mesh name not found");
	return NULL;
}

namespace mesh_resource_internal
{
	void *load(File &file, Allocator &a)
	{
		BinaryReader br(file);

		u32 version;
		br.read(version);
		CE_ASSERT(version == RESOURCE_HEADER(RESOURCE_VERSION_MESH), "Wrong version");

		u32 num_geoms;
		br.read(num_geoms);

		MeshResource *mr = CE_NEW(a, MeshResource)(a);
		array::resize(mr->geometry_names, num_geoms);
		array::resize(mr->geometries, num_geoms);

		for (u32 i = 0; i < num_geoms; ++i) {
			StringId32 name;
			br.read(name);

			bgfx::VertexLayout layout;
			BgfxReader reader(br);
			bgfx::read(&reader, layout);

			OBB obb;
			br.read(obb);

			u32 num_verts;
			br.read(num_verts);

			u32 stride;
			br.read(stride);

			u32 num_inds;
			br.read(num_inds);

			const u32 vsize = num_verts*stride;
			const u32 isize = num_inds*sizeof(u16);

			const u32 size = sizeof(MeshGeometry) + vsize + isize;

			MeshGeometry *mg = (MeshGeometry *)a.allocate(size);
			mg->obb             = obb;
			mg->layout          = layout;
			mg->vertex_buffer   = BGFX_INVALID_HANDLE;
			mg->index_buffer    = BGFX_INVALID_HANDLE;
			mg->vertices.num    = num_verts;
			mg->vertices.stride = stride;
			mg->vertices.data   = (char *)&mg[1];
			mg->indices.num     = num_inds;
			mg->indices.data    = mg->vertices.data + vsize;

			br.read(mg->vertices.data, vsize);
			br.read(mg->indices.data, isize);

			mr->geometry_names[i] = name;
			mr->geometries[i] = mg;
		}

		return mr;
	}

	void online(StringId64 id, ResourceManager &rm)
	{
		MeshResource *mr = (MeshResource *)rm.get(RESOURCE_TYPE_MESH, id);

		for (u32 i = 0; i < array::size(mr->geometries); ++i) {
			MeshGeometry &mg = *mr->geometries[i];

			const u32 vsize = mg.vertices.num * mg.vertices.stride;
			const u32 isize = mg.indices.num * sizeof(u16);

			const bgfx::Memory *vmem = bgfx::makeRef(mg.vertices.data, vsize);
			const bgfx::Memory *imem = bgfx::makeRef(mg.indices.data, isize);

			bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(vmem, mg.layout);
			bgfx::IndexBufferHandle ibh  = bgfx::createIndexBuffer(imem);
			CE_ASSERT(bgfx::isValid(vbh), "Invalid vertex buffer");
			CE_ASSERT(bgfx::isValid(ibh), "Invalid index buffer");

			mg.vertex_buffer = vbh;
			mg.index_buffer  = ibh;
		}
	}

	void offline(StringId64 id, ResourceManager &rm)
	{
		MeshResource *mr = (MeshResource *)rm.get(RESOURCE_TYPE_MESH, id);

		for (u32 i = 0; i < array::size(mr->geometries); ++i) {
			const MeshGeometry &mg = *mr->geometries[i];
			bgfx::destroy(mg.vertex_buffer);
			bgfx::destroy(mg.index_buffer);
		}
	}

	void unload(Allocator &a, void *res)
	{
		MeshResource *mr = (MeshResource *)res;

		for (u32 i = 0; i < array::size(mr->geometries); ++i) {
			a.deallocate(mr->geometries[i]);
		}
		CE_DELETE(a, (MeshResource *)res);
	}

} // namespace mesh_resource_internal

#if CROWN_CAN_COMPILE
namespace mesh_resource_internal
{
	static void parse_float_array(Array<f32> &output, const char *json)
	{
		TempAllocator4096 ta;
		JsonArray floats(ta);
		sjson::parse_array(floats, json);

		array::resize(output, array::size(floats));
		for (u32 i = 0; i < array::size(floats); ++i)
			output[i] = sjson::parse_float(floats[i]);
	}

	static void parse_index_array(Array<u16> &output, const char *json)
	{
		TempAllocator4096 ta;
		JsonArray indices(ta);
		sjson::parse_array(indices, json);

		array::resize(output, array::size(indices));
		for (u32 i = 0; i < array::size(indices); ++i)
			output[i] = (u16)sjson::parse_int(indices[i]);
	}

	namespace mesh
	{
		s32 parse_nodes(Mesh &m, const char *sjson, CompileOptions &opts);

		s32 parse_node(Node &n, const char *sjson, Mesh *mesh, CompileOptions &opts)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			sjson::parse(obj, sjson);

			n._local_pose = sjson::parse_matrix4x4(obj["matrix_local"]);

			if (json_object::has(obj, "children")) {
				s32 err = mesh::parse_nodes(*mesh, obj["children"], opts);
				DATA_COMPILER_ENSURE(err == 0, opts);
			}

			return 0;
		}

		void reset(Geometry &g)
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

		u32 vertex_stride(Geometry &g)
		{
			u32 stride = 0;
			stride += 3 * sizeof(f32);
			stride += (has_normals(g) ? 3 * sizeof(f32) : 0);
			stride += (has_uvs(g)     ? 2 * sizeof(f32) : 0);
			return stride;
		}

		bgfx::VertexLayout vertex_layout(Geometry &g)
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

		void generate_vertex_and_index_buffers(Geometry &g)
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

		OBB obb(Geometry &g)
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

		s32 parse_indices(Geometry &g, const char *json)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			sjson::parse(obj, json);

			JsonArray data_json(ta);
			sjson::parse_array(data_json, obj["data"]);

			parse_index_array(g._position_indices, data_json[0]);

			if (has_normals(g)) {
				parse_index_array(g._normal_indices, data_json[1]);
			}
			if (has_uvs(g)) {
				parse_index_array(g._uv_indices, data_json[2]);
			}

			return 0;
		}

		s32 parse_geometry(Geometry &g, const char *sjson)
		{
			TempAllocator4096 ta;
			JsonObject obj(ta);
			sjson::parse(obj, sjson);

			parse_float_array(g._positions, obj["position"]);

			if (json_object::has(obj, "normal")) {
				parse_float_array(g._normals, obj["normal"]);
			}
			if (json_object::has(obj, "texcoord")) {
				parse_float_array(g._uvs, obj["texcoord"]);
			}

			return parse_indices(g, obj["indices"]);
		}

		s32 parse_geometries(Mesh &m, const char *sjson, CompileOptions &opts)
		{
			TempAllocator4096 ta;
			JsonObject geometries(ta);
			sjson::parse(geometries, sjson);

			auto cur = json_object::begin(geometries);
			auto end = json_object::end(geometries);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(geometries, cur);

				Geometry geo(default_allocator());
				s32 err = mesh::parse_geometry(geo, cur->second);
				DATA_COMPILER_ENSURE(err == 0, opts);

				DynamicString geometry_name(ta);
				geometry_name = cur->first;
				DATA_COMPILER_ASSERT(!hash_map::has(m._geometries, geometry_name)
					, opts
					, "Geometry redefined: '%s'"
					, geometry_name.c_str()
					);
				hash_map::set(m._geometries, geometry_name, geo);
			}

			return 0;
		}

		s32 parse_nodes(Mesh &m, const char *sjson, CompileOptions &opts)
		{
			TempAllocator4096 ta;
			JsonObject nodes(ta);
			sjson::parse(nodes, sjson);

			auto cur = json_object::begin(nodes);
			auto end = json_object::end(nodes);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(nodes, cur);

				Node node(default_allocator());
				s32 err = mesh::parse_node(node, cur->second, &m, opts);
				DATA_COMPILER_ENSURE(err == 0, opts);

				DynamicString node_name(ta);
				node_name = cur->first;
				DATA_COMPILER_ASSERT(!hash_map::has(m._nodes, node_name)
					, opts
					, "Node redefined: '%s'"
					, node_name.c_str()
					);

				hash_map::set(m._nodes, node_name, node);
			}

			return 0;
		}

		s32 parse(Mesh &m, Buffer &buf, CompileOptions &opts)
		{
			TempAllocator4096 ta;
			JsonObject nodes(ta);
			JsonObject obj(ta);
			sjson::parse(obj, buf);

			s32 err = mesh::parse_geometries(m, obj["geometries"], opts);
			DATA_COMPILER_ENSURE(err == 0, opts);

			return mesh::parse_nodes(m, obj["nodes"], opts);
		}

		s32 parse(Mesh &m, CompileOptions &opts)
		{
			return mesh::parse(m, opts, opts._source_path.c_str());
		}

		s32 write(Mesh &m, CompileOptions &opts)
		{
			opts.write(RESOURCE_HEADER(RESOURCE_VERSION_MESH));
			opts.write(hash_map::size(m._geometries));

			auto cur = hash_map::begin(m._geometries);
			auto end = hash_map::end(m._geometries);
			for (; cur != end; ++cur) {
				HASH_MAP_SKIP_HOLE(m._geometries, cur);

				Geometry *geo = (Geometry *)&cur->second;
				mesh::generate_vertex_and_index_buffers(*geo);

				opts.write(cur->first.to_string_id()._id);

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

	} // namespace mesh

	namespace mesh
	{
		s32 parse(Mesh &m, CompileOptions &opts, const char *path)
		{
			Buffer buf = opts.read(path);

			if (str_has_suffix(path, ".mesh"))
				return mesh::parse(m, buf, opts);

			DATA_COMPILER_ASSERT(false
				, opts
				, "Unknown mesh '%s'"
				, path
				);
		}

	} // namespace mesh

	Node::Node(Allocator &a)
		: _local_pose(MATRIX4X4_IDENTITY)
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

	s32 compile(CompileOptions &opts)
	{
		Mesh mesh(default_allocator());
		s32 err = mesh::parse(mesh, opts);
		DATA_COMPILER_ENSURE(err == 0, opts);

		return mesh::write(mesh, opts);
	}

} // namespace mesh_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
