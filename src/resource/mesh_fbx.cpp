/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
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
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/fbx_document.h"
#include "resource/mesh_fbx.h"
#include <stdlib.h>
#include <ufbx.h>

LOG_SYSTEM(FBX_RESOURCE, "fbx_resource")

namespace crown
{
namespace fbx
{
	void generate_indices(Array<u32> &triangle_indices, Array<f32> &vertex_data, size_t vertex_size)
	{
		ufbx_vertex_stream streams[1] =
		{
			{ array::begin(vertex_data), array::size(vertex_data), vertex_size },
		};

		size_t num_vertices = ufbx_generate_indices(streams
			, 1
			, array::begin(triangle_indices)
			, array::size(triangle_indices)
			, NULL
			, NULL
			);
		array::resize(vertex_data, (u32)num_vertices * vertex_size);
	}

	/// See: https://ufbx.github.io/elements/meshes/#example
	size_t convert_mesh_part(Geometry &g, FBXDocument &fbx, const ufbx_mesh *mesh, const ufbx_mesh_part *part)
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

				// See: https://ufbx.github.io/elements/deformers/
				if (mesh->skin_deformers.count == 1) {
					ufbx_skin_deformer *skin = mesh->skin_deformers.data[0];

					uint32_t vertex = mesh->vertex_indices.data[index];
					ufbx_skin_vertex skin_vertex = skin->vertices.data[vertex];
					f32 bones[Geometry::MAX_BONE_WEIGHTS] = { 0 };
					f32 weights[Geometry::MAX_BONE_WEIGHTS] = { 0.0f };
					u32 num_weights = min(u32(skin_vertex.num_weights), u32(Geometry::MAX_BONE_WEIGHTS));

					// Read bone ID and weight.
					f32 total_weight = 0.0f;
					for (u32 i = 0; i < num_weights; i++) {
						ufbx_skin_weight skin_weight = skin->weights.data[skin_vertex.weight_begin + i];
						ufbx_skin_cluster *cluster = skin->clusters.data[skin_weight.cluster_index];

						bones[i] = (f32)fbx::bone_id(fbx, cluster->bone_node->name.data);
						weights[i] = (f32)skin_weight.weight;
						total_weight += weights[i];
					}

					// FBX does not guarantee that skin weights are normalized, and we may even
					// be dropping some, so we must renormalize them.
					for (u32 i = 0; i < num_weights; i++)
						weights[i] /= total_weight;

					for (u32 i = 0; i < countof(bones); ++i) {
						array::push_back(g._bones, bones[i]);
						array::push_back(g._weights, weights[i]);
					}
				}

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

	s32 parse_geometry(Geometry &g, FBXDocument &fbx, const ufbx_mesh *mesh)
	{
		size_t num_indices = 0;
		for (size_t i = 0; i < mesh->material_parts.count; ++i) {
			const ufbx_mesh_part *mesh_part = &mesh->material_parts.data[i];
			num_indices += convert_mesh_part(g, fbx, mesh, mesh_part);
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

		if (mesh::has_bones(g)) {
			array::resize(g._bone_indices, (u32)num_indices);
			array::resize(g._weight_indices, (u32)num_indices);
			generate_indices(g._bone_indices, g._bones, sizeof(f32)*4);
			generate_indices(g._weight_indices, g._weights, sizeof(f32)*4);
		}

		if (mesh::has_uvs(g)) {
			array::resize(g._uv_indices, (u32)num_indices);
			generate_indices(g._uv_indices, g._uvs, sizeof(f32)*2);
		}

		return 0;
	}

	s32 parse_geometries(Mesh &m, FBXDocument &fbx, const ufbx_mesh_list *meshes, CompileOptions &opts)
	{
		for (size_t i = 0; i < meshes->count; ++i) {
			const ufbx_mesh *mesh = meshes->data[i];
			Geometry geo(default_allocator());

			s32 err = fbx::parse_geometry(geo, fbx, mesh);
			ENSURE_OR_RETURN(err == 0, opts);

			DynamicString geometry_name(default_allocator());
			geometry_name.from_string_id(StringId32((const char *)&mesh, sizeof(mesh)));

			RETURN_IF_FALSE(!hash_map::has(m._geometries, geometry_name)
				, opts
				, "Geometry redefined: '%s'"
				, geometry_name.c_str()
				);
			hash_map::set(m._geometries, geometry_name, geo);
		}

		return 0;
	}

	s32 parse_node(Node &n, const ufbx_node *node)
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

	s32 parse_nodes(Mesh &m, const ufbx_node_list *nodes, CompileOptions &opts)
	{
		for (size_t i = 0; i < nodes->count; ++i) {
			const ufbx_node *node = nodes->data[i];

			Node new_node(default_allocator());

			DynamicString node_name(default_allocator());
			node_name.set(node->name.data, node->name.length);

			s32 err = fbx::parse_node(new_node, node);
			ENSURE_OR_RETURN(err == 0, opts);

			hash_map::set(m._nodes, node_name, new_node);
		}

		return 0;
	}

	s32 parse(Mesh &m, Buffer &buf, CompileOptions &opts)
	{
		FBXDocument fbx(default_allocator());
		s32 err = fbx::parse(fbx, buf, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		err = parse_geometries(m, fbx, &fbx.scene->meshes, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		return parse_nodes(m, &fbx.scene->nodes, opts);
	}

} // namespace fbx

} // namespace crown

#endif // if CROWN_CAN_COMPILE
