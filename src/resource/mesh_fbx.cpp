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
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/fbx_document.h"
#include "resource/mesh_fbx.h"
#include <stb_sprintf.h>
#include <ufbx.h>

LOG_SYSTEM(FBX_RESOURCE, "fbx_resource")

namespace crown
{
namespace fbx
{
	/// See: https://ufbx.github.io/elements/meshes/#example
	size_t convert_mesh_part(Geometry &g
		, const ufbx_mesh *mesh
		, const ufbx_mesh_part *part
		, u32 *triangle_indices
		, size_t num_triangle_indices
		)
	{
		size_t num_triangles = part->num_triangles;

		// Iterate over each face using the specific material.
		for (size_t face_ix = 0; face_ix < part->num_faces; face_ix++) {
			ufbx_face face = mesh->faces.data[part->face_indices.data[face_ix]];

			// Triangulate the face into `triangle_indices[]`.
			uint32_t num_tris = ufbx_triangulate_face(triangle_indices, num_triangle_indices, mesh, face);

			// Iterate over each triangle corner contiguously.
			for (size_t i = 0; i < num_tris * 3; i++) {
				uint32_t index = triangle_indices[i];
				array::push_back(g._position_indices, mesh->vertex_position.indices.data[index]);

				if (mesh->vertex_normal.exists)
					array::push_back(g._normal_indices, mesh->vertex_normal.indices.data[index]);

				if (mesh->vertex_tangent.exists)
					array::push_back(g._tangent_indices, mesh->vertex_tangent.indices.data[index]);

				if (mesh->vertex_bitangent.exists)
					array::push_back(g._bitangent_indices, mesh->vertex_bitangent.indices.data[index]);

				if (mesh->skin_deformers.count == 1) {
					array::push_back(g._bone_indices, mesh->vertex_indices.data[index]);
					array::push_back(g._weight_indices, mesh->vertex_indices.data[index]);
				}

				if (mesh->vertex_uv.exists)
					array::push_back(g._uv_indices, mesh->vertex_uv.indices.data[index]);
			}
		}

		return num_triangles * 3;
	}

	static void copy_vertex_vec3(Array<f32> &values
		, u32 values_offset
		, Array<u32> &indices
		, u32 indices_offset
		, const ufbx_vertex_vec3 &attribute
		)
	{
		TempAllocator4096 ta;
		Array<u32> remap(ta);
		array::resize(remap, (u32)attribute.values.count);
		for (u32 i = 0; i < array::size(remap); ++i)
			remap[i] = UINT32_MAX;

		u32 num_values = 0;
		for (u32 i = indices_offset; i < array::size(indices); ++i) {
			const u32 source_index = indices[i];
			CE_ENSURE(source_index < array::size(remap));
			if (remap[source_index] == UINT32_MAX)
				remap[source_index] = num_values++;
			indices[i] = remap[source_index];
		}

		array::reserve(values, values_offset + num_values * 3);
		array::resize(values, values_offset + num_values * 3);
		for (u32 i = 0; i < array::size(remap); ++i) {
			if (remap[i] == UINT32_MAX)
				continue;
			const ufbx_vec3 v = attribute.values.data[i];
			const u32 offset = values_offset + remap[i] * 3;
			values[offset + 0] = (f32)v.x;
			values[offset + 1] = (f32)v.y;
			values[offset + 2] = (f32)v.z;
		}
	}

	static void copy_vertex_uv(Array<f32> &values
		, u32 values_offset
		, Array<u32> &indices
		, u32 indices_offset
		, const ufbx_vertex_vec2 &attribute
		)
	{
		TempAllocator4096 ta;
		Array<u32> remap(ta);
		array::resize(remap, (u32)attribute.values.count);
		for (u32 i = 0; i < array::size(remap); ++i)
			remap[i] = UINT32_MAX;

		u32 num_values = 0;
		for (u32 i = indices_offset; i < array::size(indices); ++i) {
			const u32 source_index = indices[i];
			CE_ENSURE(source_index < array::size(remap));
			if (remap[source_index] == UINT32_MAX)
				remap[source_index] = num_values++;
			indices[i] = remap[source_index];
		}

		array::reserve(values, values_offset + num_values * 2);
		array::resize(values, values_offset + num_values * 2);
		for (u32 i = 0; i < array::size(remap); ++i) {
			if (remap[i] == UINT32_MAX)
				continue;
			const ufbx_vec2 v = attribute.values.data[i];
			const u32 offset = values_offset + remap[i] * 2;
			values[offset + 0] =        (f32)v.x;
			values[offset + 1] = 1.0f - (f32)v.y;
		}
	}

	static void copy_skin(Geometry &g
		, const GeometryInfo &geometry
		, FBXDocument &fbx
		, const ufbx_mesh *mesh
		)
	{
		TempAllocator4096 ta;
		Array<u32> remap(ta);
		array::resize(remap, (u32)mesh->num_vertices);
		for (u32 i = 0; i < array::size(remap); ++i)
			remap[i] = UINT32_MAX;

		u32 num_vertices = 0;
		for (u32 i = 0; i < array::size(g._bone_indices) - geometry._bone_indices.offset; ++i) {
			const u32 bone_index = geometry._bone_indices.offset + i;
			const u32 weight_index = geometry._weight_indices.offset + i;
			const u32 source_index = g._bone_indices[bone_index];
			CE_ENSURE(source_index < array::size(remap));
			if (remap[source_index] == UINT32_MAX)
				remap[source_index] = num_vertices++;
			g._bone_indices[bone_index] = remap[source_index];
			g._weight_indices[weight_index] = remap[source_index];
		}

		array::reserve(g._bones, geometry._bones.offset + num_vertices * Geometry::MAX_BONE_WEIGHTS);
		array::resize(g._bones, geometry._bones.offset + num_vertices * Geometry::MAX_BONE_WEIGHTS);
		array::reserve(g._weights, geometry._weights.offset + num_vertices * Geometry::MAX_BONE_WEIGHTS);
		array::resize(g._weights, geometry._weights.offset + num_vertices * Geometry::MAX_BONE_WEIGHTS);
		const ufbx_skin_deformer *skin = mesh->skin_deformers.data[0];
		for (u32 vertex = 0; vertex < array::size(remap); ++vertex) {
			if (remap[vertex] == UINT32_MAX)
				continue;

			const ufbx_skin_vertex skin_vertex = skin->vertices.data[vertex];
			f32 bones[Geometry::MAX_BONE_WEIGHTS] = { 0 };
			f32 weights[Geometry::MAX_BONE_WEIGHTS] = { 0.0f };
			const u32 num_weights = min(u32(skin_vertex.num_weights), u32(Geometry::MAX_BONE_WEIGHTS));

			// Read bone ID and weight.
			f32 total_weight = 0.0f;
			for (u32 i = 0; i < num_weights; ++i) {
				const ufbx_skin_weight skin_weight = skin->weights.data[skin_vertex.weight_begin + i];
				const ufbx_skin_cluster *cluster = skin->clusters.data[skin_weight.cluster_index];
				bones[i] = (f32)fbx::bone_id(fbx, cluster->bone_node->name.data);
				weights[i] = (f32)skin_weight.weight;
				total_weight += weights[i];
			}

			// FBX does not guarantee that skin weights are normalized, and we may even
			// be dropping some, so we must renormalize them.
			for (u32 i = 0; i < num_weights; ++i)
				weights[i] /= total_weight;

			const u32 offset = remap[vertex] * Geometry::MAX_BONE_WEIGHTS;
			for (u32 i = 0; i < countof(bones); ++i) {
				g._bones[geometry._bones.offset + offset + i] = bones[i];
				g._weights[geometry._weights.offset + offset + i] = weights[i];
			}
		}
	}

	s32 parse_geometry(Geometry &g, GeometryInfo &geometry, FBXDocument &fbx, const ufbx_mesh *mesh)
	{
		TempAllocator4096 ta;
		HashMap<DynamicString, bool> used_slots(ta);
		geometry._positions.offset = array::size(g._positions);
		geometry._normals.offset = array::size(g._normals);
		geometry._uvs.offset = array::size(g._uvs);
		geometry._tangents.offset = array::size(g._tangents);
		geometry._bitangents.offset = array::size(g._bitangents);
		geometry._bones.offset = array::size(g._bones);
		geometry._weights.offset = array::size(g._weights);
		geometry._position_indices.offset = array::size(g._position_indices);
		geometry._normal_indices.offset = array::size(g._normal_indices);
		geometry._tangent_indices.offset = array::size(g._tangent_indices);
		geometry._bitangent_indices.offset = array::size(g._bitangent_indices);
		geometry._bone_indices.offset = array::size(g._bone_indices);
		geometry._weight_indices.offset = array::size(g._weight_indices);
		geometry._uv_indices.offset = array::size(g._uv_indices);
		geometry._material_ranges.offset = array::size(g._material_ranges);

		const u32 expected_num_indices = (u32)mesh->num_triangles * 3;
		array::reserve(g._position_indices, geometry._position_indices.offset + expected_num_indices);
		if (mesh->vertex_normal.exists)
			array::reserve(g._normal_indices, geometry._normal_indices.offset + expected_num_indices);
		if (mesh->vertex_tangent.exists)
			array::reserve(g._tangent_indices, geometry._tangent_indices.offset + expected_num_indices);
		if (mesh->vertex_bitangent.exists)
			array::reserve(g._bitangent_indices, geometry._bitangent_indices.offset + expected_num_indices);
		if (mesh->skin_deformers.count == 1) {
			array::reserve(g._bone_indices, geometry._bone_indices.offset + expected_num_indices);
			array::reserve(g._weight_indices, geometry._weight_indices.offset + expected_num_indices);
		}
		if (mesh->vertex_uv.exists)
			array::reserve(g._uv_indices, geometry._uv_indices.offset + expected_num_indices);

		Array<u32> triangle_indices(ta);
		array::resize(triangle_indices, (u32)mesh->max_face_triangles * 3);
		size_t num_indices = 0;
		for (size_t i = 0; i < mesh->material_parts.count; ++i) {
			const ufbx_mesh_part *mesh_part = &mesh->material_parts.data[i];
			// Name empty parts too so slot disambiguation matches the importer.
			char suffix[32];
			stbsp_snprintf(suffix, sizeof(suffix), "_%u", mesh_part->index);
			DynamicString slot(ta);
			if (mesh->materials.count == 0) {
				slot = "default";
			} else if (mesh->materials.data[mesh_part->index]->name.length != 0) {
				slot = mesh->materials.data[mesh_part->index]->name.data;
			} else {
				slot = "material";
				slot += suffix;
			}
			while (hash_map::has(used_slots, slot))
				slot += suffix;
			hash_map::set(used_slots, slot, true);

			const u32 count = (u32)convert_mesh_part(g
				, mesh
				, mesh_part
				, array::begin(triangle_indices)
				, array::size(triangle_indices)
				);
			if (count != 0)
				array::push_back(g._material_ranges, { slot.to_string_id(), (u32)num_indices, count });
			num_indices += count;
		}

		copy_vertex_vec3(g._positions
			, geometry._positions.offset
			, g._position_indices
			, geometry._position_indices.offset
			, mesh->vertex_position
			);
		if (mesh->vertex_normal.exists)
			copy_vertex_vec3(g._normals
				, geometry._normals.offset
				, g._normal_indices
				, geometry._normal_indices.offset
				, mesh->vertex_normal
				);
		if (mesh->vertex_tangent.exists)
			copy_vertex_vec3(g._tangents
				, geometry._tangents.offset
				, g._tangent_indices
				, geometry._tangent_indices.offset
				, mesh->vertex_tangent
				);
		if (mesh->vertex_bitangent.exists)
			copy_vertex_vec3(g._bitangents
				, geometry._bitangents.offset
				, g._bitangent_indices
				, geometry._bitangent_indices.offset
				, mesh->vertex_bitangent
				);
		if (mesh->skin_deformers.count == 1)
			copy_skin(g, geometry, fbx, mesh);
		if (mesh->vertex_uv.exists)
			copy_vertex_uv(g._uvs
				, geometry._uvs.offset
				, g._uv_indices
				, geometry._uv_indices.offset
				, mesh->vertex_uv
				);

		geometry._positions.count = array::size(g._positions) - geometry._positions.offset;
		geometry._normals.count = array::size(g._normals) - geometry._normals.offset;
		geometry._uvs.count = array::size(g._uvs) - geometry._uvs.offset;
		geometry._tangents.count = array::size(g._tangents) - geometry._tangents.offset;
		geometry._bitangents.count = array::size(g._bitangents) - geometry._bitangents.offset;
		geometry._bones.count = array::size(g._bones) - geometry._bones.offset;
		geometry._weights.count = array::size(g._weights) - geometry._weights.offset;
		geometry._position_indices.count = array::size(g._position_indices) - geometry._position_indices.offset;
		geometry._normal_indices.count = array::size(g._normal_indices) - geometry._normal_indices.offset;
		geometry._tangent_indices.count = array::size(g._tangent_indices) - geometry._tangent_indices.offset;
		geometry._bitangent_indices.count = array::size(g._bitangent_indices) - geometry._bitangent_indices.offset;
		geometry._bone_indices.count = array::size(g._bone_indices) - geometry._bone_indices.offset;
		geometry._weight_indices.count = array::size(g._weight_indices) - geometry._weight_indices.offset;
		geometry._uv_indices.count = array::size(g._uv_indices) - geometry._uv_indices.offset;
		geometry._material_ranges.count = array::size(g._material_ranges) - geometry._material_ranges.offset;

		return 0;
	}

	s32 parse_geometries(Mesh &m, FBXDocument &fbx, const ufbx_mesh_list *meshes, CompileOptions &opts)
	{
		for (size_t i = 0; i < meshes->count; ++i) {
			const ufbx_mesh *mesh = meshes->data[i];
			GeometryInfo geo = {};

			if (mesh->num_triangles == 0)
				continue;

			s32 err = fbx::parse_geometry(m._geometry, geo, fbx, mesh);
			ENSURE_OR_RETURN(FBX_RESOURCE, err == 0, opts);

			DynamicString geometry_name(default_allocator());
			geometry_name.from_string_id(StringId32((const char *)&mesh, sizeof(mesh)));

			RETURN_IF_FALSE(FBX_RESOURCE, !hash_map::has(m._geometries, geometry_name)
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
			node_name.set(node->name.data, (u32)node->name.length);

			s32 err = fbx::parse_node(new_node, node);
			ENSURE_OR_RETURN(FBX_RESOURCE, err == 0, opts);

			hash_map::set(m._nodes, node_name, new_node);
		}

		return 0;
	}

	s32 parse(Mesh &m, Buffer &buf, CompileOptions &opts)
	{
		FBXDocument fbx(default_allocator());
		s32 err = fbx::parse(fbx, buf, opts);
		ENSURE_OR_RETURN(FBX_RESOURCE, err == 0, opts);

		err = parse_geometries(m, fbx, &fbx.scene->meshes, opts);
		ENSURE_OR_RETURN(FBX_RESOURCE, err == 0, opts);

		return parse_nodes(m, &fbx.scene->nodes, opts);
	}

} // namespace fbx

} // namespace crown

#endif // if CROWN_CAN_COMPILE
