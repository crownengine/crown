/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
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
#include "resource/mesh.h"
#include "resource/mesh_resource.h"
#include "resource/resource_manager.h"
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

MeshResource::MeshResource(Allocator &a)
	: nodes(a)
	, geometries(a)
{
}

const MeshGeometry *MeshResource::geometry(StringId32 name) const
{
	for (u32 i = 0; i < array::size(nodes); ++i) {
		if (nodes[i].name == name)
			return geometries[nodes[i].geometry_index];
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

		for (u32 i = 0; i < num_geoms; ++i) {
			u32 num_names;
			br.read(num_names);

			for (u32 j = 0; j < num_names; ++j) {
				StringId32 name;
				br.read(name);

				array::push_back(mr->nodes, { name, i });
			}

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

			array::push_back(mr->geometries, mg);
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
namespace mesh
{
	static s32 parse_float_array(Array<f32> &output, const char *json, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonArray floats(ta);
		RETURN_IF_ERROR(sjson::parse_array(floats, json), opts);

		array::resize(output, array::size(floats));
		for (u32 i = 0; i < array::size(floats); ++i) {
			output[i] = RETURN_IF_ERROR(sjson::parse_float(floats[i]), opts);
		}

		return 0;
	}

	static s32 parse_index_array(Array<u32> &output, const char *json, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonArray indices(ta);
		RETURN_IF_ERROR(sjson::parse_array(indices, json), opts);

		array::resize(output, array::size(indices));
		for (u32 i = 0; i < array::size(indices); ++i) {
			output[i] = RETURN_IF_ERROR(sjson::parse_int(indices[i]), opts);
		}

		return 0;
	}

	s32 parse_nodes(Mesh &m, const char *sjson, CompileOptions &opts);

	s32 parse_node(Node &n, const char *sjson, Mesh *mesh, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, sjson), opts);

		n._local_pose = RETURN_IF_ERROR(sjson::parse_matrix4x4(obj["matrix_local"]), opts);

		if (json_object::has(obj, "children")) {
			s32 err = mesh::parse_nodes(*mesh, obj["children"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		if (json_object::has(obj, "geometry")) {
			RETURN_IF_ERROR(sjson::parse_string(n._geometry, obj["geometry"]), opts);
		}

		return 0;
	}

	s32 parse_indices(Geometry &g, const char *json, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, json), opts);

		JsonArray data_json(ta);
		RETURN_IF_ERROR(sjson::parse_array(data_json, obj["data"]), opts);

		parse_index_array(g._position_indices, data_json[0], opts);

		u32 idx = 1;

		if (has_normals(g))
			parse_index_array(g._normal_indices, data_json[idx++], opts);

		if (has_uvs(g))
			parse_index_array(g._uv_indices, data_json[idx++], opts);

		if (has_tangents(g))
			parse_index_array(g._tangent_indices, data_json[idx++], opts);

		if (has_bitangents(g))
			parse_index_array(g._bitangent_indices, data_json[idx++], opts);

		return 0;
	}

	s32 parse_geometry(Geometry &g, const char *sjson, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, sjson), opts);

		s32 err = parse_float_array(g._positions, obj["position"], opts);
		ENSURE_OR_RETURN(err == 0, opts);

		if (json_object::has(obj, "normal")) {
			err = parse_float_array(g._normals, obj["normal"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		if (json_object::has(obj, "tangent")) {
			err = parse_float_array(g._tangents, obj["tangent"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		if (json_object::has(obj, "bitangent")) {
			err = parse_float_array(g._bitangents, obj["bitangent"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		if (json_object::has(obj, "texcoord")) {
			err = parse_float_array(g._uvs, obj["texcoord"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		return parse_indices(g, obj["indices"], opts);
	}

	s32 parse_geometries(Mesh &m, const char *sjson, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject geometries(ta);
		RETURN_IF_ERROR(sjson::parse(geometries, sjson), opts);

		auto cur = json_object::begin(geometries);
		auto end = json_object::end(geometries);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(geometries, cur);

			Geometry geo(default_allocator());
			s32 err = mesh::parse_geometry(geo, cur->second, opts);
			ENSURE_OR_RETURN(err == 0, opts);

			DynamicString geometry_name(ta);
			geometry_name = cur->first;
			RETURN_IF_FALSE(!hash_map::has(m._geometries, geometry_name)
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
		RETURN_IF_ERROR(sjson::parse(nodes, sjson), opts);

		auto cur = json_object::begin(nodes);
		auto end = json_object::end(nodes);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(nodes, cur);

			Node node(default_allocator());
			s32 err = mesh::parse_node(node, cur->second, &m, opts);
			ENSURE_OR_RETURN(err == 0, opts);

			DynamicString node_name(ta);
			node_name = cur->first;
			RETURN_IF_FALSE(!hash_map::has(m._nodes, node_name)
				, opts
				, "Node redefined: '%s'"
				, node_name.c_str()
				);

			// For backwards compatibility: originally .mesh resources
			// enforced (implicitly) a 1:1 relationship between nodes
			// and geometries.
			if (node._geometry == "")
				node._geometry = node_name;

			RETURN_IF_FALSE(hash_map::has(m._geometries, node._geometry)
				, opts
				, "Node '%s' references unexisting geometry '%s'"
				, node_name.c_str()
				, node._geometry.c_str()
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
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		s32 err = mesh::parse_geometries(m, obj["geometries"], opts);
		ENSURE_OR_RETURN(err == 0, opts);

		return mesh::parse_nodes(m, obj["nodes"], opts);
	}

} // namespace mesh

namespace mesh_resource_internal
{
	s32 compile(CompileOptions &opts)
	{
		Mesh mesh(default_allocator());
		s32 err = mesh::parse(mesh, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		return mesh::write(mesh, opts);
	}

} // namespace mesh_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
