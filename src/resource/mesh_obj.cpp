/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/math/matrix4x4.inl"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/mesh_obj.h"
#include <stdlib.h>
#include <string.h>
#include <ufbx.h>

LOG_SYSTEM(OBJ_RESOURCE, "obj_resource")

namespace crown
{
namespace obj
{
	struct OBJDocument
	{
		ufbx_scene *scene;

		///
		OBJDocument()
			: scene(NULL)
		{
		}

		///
		~OBJDocument()
		{
			ufbx_free_scene(scene);
		}
	};

	static void setup_load_opts(ufbx_load_opts &load_opts, const char *filename)
	{
		load_opts.file_format = UFBX_FILE_FORMAT_OBJ;
		load_opts.target_axes = ufbx_axes_right_handed_z_up;
		load_opts.target_unit_meters = 1.0f;
		load_opts.space_conversion = UFBX_SPACE_CONVERSION_TRANSFORM_ROOT;
		load_opts.obj_axes = ufbx_axes_right_handed_z_up;
		load_opts.obj_unit_meters = 1.0f;
		load_opts.obj_search_mtl_by_filename = true;
		load_opts.ignore_missing_external_files = true;

		if (filename != NULL) {
			load_opts.filename.data = filename;
			load_opts.filename.length = strlen(filename);
		}
	}

	static s32 parse_scene(OBJDocument &obj, Buffer &buf, CompileOptions &opts, const char *filename)
	{
		ufbx_load_opts load_opts = {};
		setup_load_opts(load_opts, filename);

		ufbx_error error;
		obj.scene = ufbx_load_memory(array::begin(buf)
			, array::size(buf)
			, &load_opts
			, &error
			);
		RETURN_IF_FALSE(OBJ_RESOURCE, obj.scene != NULL
			, opts
			, "ufbx: %s"
			, error.description.data
			);

		return 0;
	}

	static void generate_indices(Array<u32> &triangle_indices, Array<f32> &vertex_data, size_t vertex_size)
	{
		ufbx_vertex_stream streams[1] =
		{
			{ array::begin(vertex_data), array::size(vertex_data), vertex_size },
		};

		ufbx_generate_indices(streams
			, 1
			, array::begin(triangle_indices)
			, array::size(triangle_indices)
			, NULL
			, NULL
			);
	}

	/// See: https://ufbx.github.io/elements/meshes/#example
	static size_t convert_mesh_part(Geometry &g, const ufbx_mesh *mesh, const ufbx_mesh_part *part)
	{
		size_t num_triangles = part->num_triangles;

		// Reserve space for the maximum triangle indices.
		size_t num_tri_indices = mesh->max_face_triangles * 3;
		uint32_t *tri_indices = (uint32_t *)calloc(num_tri_indices, sizeof(uint32_t));

		// Iterate over each face using the specific material.
		for (size_t face_ix = 0; face_ix < part->num_faces; face_ix++) {
			ufbx_face face = mesh->faces.data[part->face_indices.data[face_ix]];

			// Triangulate the face into `tri_indices[]`.
			uint32_t num_tris = ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);

			// Iterate over each triangle corner contiguously.
			for (size_t i = 0; i < num_tris * 3; i++) {
				uint32_t index = tri_indices[i];

				ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh->vertex_position, index);
				array::push_back(g._positions, (f32)v.x);
				array::push_back(g._positions, (f32)v.y);
				array::push_back(g._positions, (f32)v.z);

				if (mesh->vertex_normal.exists) {
					ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh->vertex_normal, index);
					array::push_back(g._normals, (f32)v.x);
					array::push_back(g._normals, (f32)v.y);
					array::push_back(g._normals, (f32)v.z);
				}

				if (mesh->vertex_tangent.exists) {
					ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh->vertex_tangent, index);
					array::push_back(g._tangents, (f32)v.x);
					array::push_back(g._tangents, (f32)v.y);
					array::push_back(g._tangents, (f32)v.z);
				}

				if (mesh->vertex_bitangent.exists) {
					ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh->vertex_bitangent, index);
					array::push_back(g._bitangents, (f32)v.x);
					array::push_back(g._bitangents, (f32)v.y);
					array::push_back(g._bitangents, (f32)v.z);
				}

				if (mesh->vertex_uv.exists) {
					ufbx_vec2 v = ufbx_get_vertex_vec2(&mesh->vertex_uv, index);
					array::push_back(g._uvs,        (f32)v.x);
					array::push_back(g._uvs, 1.0f - (f32)v.y);
				}
			}
		}

		free(tri_indices);

		return num_triangles * 3;
	}

	static s32 parse_geometry(Geometry &g, const ufbx_mesh *mesh)
	{
		size_t num_indices = 0;
		for (size_t i = 0; i < mesh->material_parts.count; ++i) {
			const ufbx_mesh_part *mesh_part = &mesh->material_parts.data[i];
			num_indices += convert_mesh_part(g, mesh, mesh_part);
		}

		array::resize(g._position_indices, (u32)num_indices);
		generate_indices(g._position_indices, g._positions, sizeof(f32)*3);

		if (mesh::has_normals(g)) {
			array::resize(g._normal_indices, (u32)num_indices);
			generate_indices(g._normal_indices, g._normals, sizeof(f32)*3);
		}

		if (mesh::has_tangents(g)) {
			array::resize(g._tangent_indices, (u32)num_indices);
			generate_indices(g._tangent_indices, g._tangents, sizeof(f32)*3);
		}

		if (mesh::has_bitangents(g)) {
			array::resize(g._bitangent_indices, (u32)num_indices);
			generate_indices(g._bitangent_indices, g._bitangents, sizeof(f32)*3);
		}

		if (mesh::has_uvs(g)) {
			array::resize(g._uv_indices, (u32)num_indices);
			generate_indices(g._uv_indices, g._uvs, sizeof(f32)*2);
		}

		return 0;
	}

	static s32 parse_geometries(Mesh &m, const ufbx_mesh_list *meshes, CompileOptions &opts)
	{
		for (size_t i = 0; i < meshes->count; ++i) {
			const ufbx_mesh *mesh = meshes->data[i];
			Geometry geo(default_allocator());

			if (mesh->num_triangles == 0)
				continue;

			s32 err = parse_geometry(geo, mesh);
			ENSURE_OR_RETURN(OBJ_RESOURCE, err == 0, opts);

			DynamicString geometry_name(default_allocator());
			geometry_name.from_string_id(StringId32((const char *)&mesh, sizeof(mesh)));

			RETURN_IF_FALSE(OBJ_RESOURCE, !hash_map::has(m._geometries, geometry_name)
				, opts
				, "Geometry redefined: '%s'"
				, geometry_name.c_str()
				);
			hash_map::set(m._geometries, geometry_name, geo);
		}

		return 0;
	}

	static s32 parse_node(Node &n, const ufbx_node *node)
	{
		Vector3 pos;
		pos.x = (f32)node->local_transform.translation.x;
		pos.y = (f32)node->local_transform.translation.y;
		pos.z = (f32)node->local_transform.translation.z;

		Quaternion rot;
		rot.x = (f32)node->local_transform.rotation.x;
		rot.y = (f32)node->local_transform.rotation.y;
		rot.z = (f32)node->local_transform.rotation.z;
		rot.w = (f32)node->local_transform.rotation.w;

		Vector3 scl;
		scl.x = (f32)node->local_transform.scale.x;
		scl.y = (f32)node->local_transform.scale.y;
		scl.z = (f32)node->local_transform.scale.z;

		n._local_pose = from_quaternion_translation(rot, pos);
		set_scale(n._local_pose, scl);

		if (node->mesh != NULL)
			n._geometry.from_string_id(StringId32((const char *)&node->mesh, sizeof(node->mesh)));

		return 0;
	}

	static s32 parse_nodes(Mesh &m, const ufbx_node_list *nodes, CompileOptions &opts)
	{
		for (size_t i = 0; i < nodes->count; ++i) {
			const ufbx_node *node = nodes->data[i];

			Node new_node(default_allocator());

			DynamicString node_name(default_allocator());
			node_name.set(node->name.data, (u32)node->name.length);

			s32 err = parse_node(new_node, node);
			ENSURE_OR_RETURN(OBJ_RESOURCE, err == 0, opts);

			hash_map::set(m._nodes, node_name, new_node);
		}

		return 0;
	}

	static s32 parse_internal(Mesh &m, Buffer &buf, CompileOptions &opts, const char *filename)
	{
		OBJDocument obj;
		s32 err = parse_scene(obj, buf, opts, filename);
		ENSURE_OR_RETURN(OBJ_RESOURCE, err == 0, opts);

		err = parse_geometries(m, &obj.scene->meshes, opts);
		ENSURE_OR_RETURN(OBJ_RESOURCE, err == 0, opts);

		return parse_nodes(m, &obj.scene->nodes, opts);
	}

	s32 parse(Mesh &m, Buffer &buf, CompileOptions &opts)
	{
		return parse_internal(m, buf, opts, NULL);
	}

	s32 parse(Mesh &m, const char *path, CompileOptions &opts)
	{
		RETURN_IF_FILE_MISSING(OBJ_RESOURCE, path, opts);
		Buffer buf = opts.read(path);
		return parse_internal(m, buf, opts, path);
	}

} // namespace obj

} // namespace crown

#endif // if CROWN_CAN_COMPILE
