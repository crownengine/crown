/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_gltf.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/containers/hash_map.inl"
#   include "core/math/matrix4x4.inl"
#   include "core/math/vector3.inl"
#   include "core/memory/globals.h"
#   include "core/memory/temp_allocator.inl"
#   include "core/strings/dynamic_string.inl"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/gltf_document.h"
#   include <cgltf.h>
#   include <stb_sprintf.h>

LOG_SYSTEM(MESH_GLTF, "mesh_gltf")

namespace crown
{
namespace gltf
{
	struct RigidAttachment
	{
		cgltf_node *node;
		cgltf_skin *skin;
		cgltf_node *bone;
		cgltf_node *root;
	};

	static bool ancestor_or_self(const cgltf_node *ancestor, const cgltf_node *node)
	{
		while (node != NULL) {
			if (node == ancestor)
				return true;
			node = node->parent;
		}
		return false;
	}

	static const RigidAttachment *rigid_attachment(const Array<RigidAttachment> &attachments
		, const cgltf_node *node
		)
	{
		for (u32 i = 0; i < array::size(attachments); ++i) {
			if (attachments[i].node == node)
				return &attachments[i];
		}
		return NULL;
	}

	static void collect_active_skin_groups(Array<u32> &groups, const GLTFDocument &doc)
	{
		for (cgltf_size i = 0; i < doc.data->nodes_count; ++i) {
			const cgltf_node *node = &doc.data->nodes[i];
			if (node->skin == NULL || !node_in_scene(doc, node))
				continue;

			const u32 group = skin_group(doc, node->skin);
			bool active = false;
			for (u32 gi = 0; gi < array::size(groups); ++gi)
				active = active || groups[gi] == group;
			if (!active)
				array::push_back(groups, group);
		}

		if (array::empty(groups)) {
			for (u32 group = 0; group < array::size(doc.skin_group_primaries); ++group)
				array::push_back(groups, group);
		}
	}

	static cgltf_node *skeleton_root(const cgltf_skin &skin)
	{
		if (skin.skeleton != NULL)
			return skin.skeleton;
		if (skin.joints_count == 0)
			return NULL;

		cgltf_node *candidate = skin.joints[0];
		while (candidate != NULL) {
			bool common = true;
			for (cgltf_size i = 1; i < skin.joints_count; ++i) {
				if (!ancestor_or_self(candidate, skin.joints[i])) {
					common = false;
					break;
				}
			}
			if (common)
				return candidate;
			candidate = candidate->parent;
		}
		return NULL;
	}

	static bool node_in_skeleton(const cgltf_skin &skin, const cgltf_node *node)
	{
		const cgltf_node *root = skeleton_root(skin);
		if (root == NULL)
			return false;

		for (cgltf_size i = 0; i < skin.joints_count; ++i) {
			const cgltf_node *ancestor = skin.joints[i];
			while (ancestor != NULL) {
				if (ancestor == node)
					return true;
				if (ancestor == root)
					break;
				ancestor = ancestor->parent;
			}
		}
		return false;
	}

	static s32 find_rigid_attachments(Array<RigidAttachment> &attachments
		, GLTFDocument &doc
		, const Array<u32> &active_groups
		, CompileOptions &opts
		)
	{
		for (cgltf_size i = 0; i < doc.data->nodes_count; ++i) {
			cgltf_node *node = &doc.data->nodes[i];
			if (node->mesh == NULL || node->skin != NULL || !node_in_scene(doc, node))
				continue;

			cgltf_node *bone = node;
			while (bone != NULL) {
				u32 selected_group = UINT32_MAX;
				for (u32 ai = 0; ai < array::size(active_groups); ++ai) {
					const u32 group = active_groups[ai];
					cgltf_skin *skin = &doc.data->skins[doc.skin_group_primaries[group]];
					if (!node_in_skeleton(*skin, bone))
						continue;
					RETURN_IF_FALSE(MESH_GLTF
						, selected_group == UINT32_MAX || selected_group == group
						, opts
						, "glTF rigid mesh '%s' is parented to bone '%s' shared by multiple armatures"
						, node_name(doc, node)
						, node_name(doc, bone)
						);
					selected_group = group;
				}

				if (selected_group != UINT32_MAX) {
					RigidAttachment attachment;
					attachment.node = node;
					attachment.skin = &doc.data->skins[doc.skin_group_primaries[selected_group]];
					attachment.bone = bone;
					attachment.root = NULL;
					array::push_back(attachments, attachment);
					break;
				}
				bone = bone->parent;
			}
		}
		return 0;
	}

	static u32 collect_subtree_skin_groups(Array<u32> &subtree_groups
		, Array<u8> &mixed_subtrees
		, const GLTFDocument &doc
		, const Array<RigidAttachment> &attachments
		, const cgltf_node *node
		)
	{
		const RigidAttachment *attachment = rigid_attachment(attachments, node);
		u32 selected = node->skin != NULL
			? skin_group(doc, node->skin)
			: (attachment != NULL ? skin_group(doc, attachment->skin) : UINT32_MAX)
			;
		bool mixed = false;
		for (cgltf_size i = 0; i < node->children_count; ++i) {
			const cgltf_node *child = node->children[i];
			const u32 child_group = collect_subtree_skin_groups(subtree_groups
				, mixed_subtrees
				, doc
				, attachments
				, child
				);
			mixed = mixed || mixed_subtrees[(u32)cgltf_node_index(doc.data, child)] != 0;
			if (child_group == UINT32_MAX)
				continue;
			if (selected != UINT32_MAX && selected != child_group)
				mixed = true;
			else
				selected = child_group;
		}

		const u32 index = (u32)cgltf_node_index(doc.data, node);
		subtree_groups[index] = selected;
		mixed_subtrees[index] = mixed;
		return selected;
	}

	static void mark_state_machine_roots(Array<u32> &state_machine_groups
		, const Array<u32> &subtree_groups
		, const Array<u8> &mixed_subtrees
		, const GLTFDocument &doc
		, const cgltf_node *node
		, u32 inherited
		)
	{
		const u32 index = (u32)cgltf_node_index(doc.data, node);
		const u32 selected = subtree_groups[index];
		u32 covered = inherited;
		if (!mixed_subtrees[index] && selected != UINT32_MAX && selected != inherited) {
			state_machine_groups[index] = selected;
			covered = selected;
		}
		for (cgltf_size i = 0; i < node->children_count; ++i) {
			mark_state_machine_roots(state_machine_groups
				, subtree_groups
				, mixed_subtrees
				, doc
				, node->children[i]
				, covered
				);
		}
	}

	static s32 resolve_rigid_attachment_roots(Array<RigidAttachment> &attachments
		, const GLTFDocument &doc
		, const Array<u32> &active_groups
		, CompileOptions &opts
		)
	{
		const cgltf_scene *active_scene = scene(doc);
		Array<u32> subtree_groups(default_allocator());
		Array<u32> state_machine_groups(default_allocator());
		Array<u8> mixed_subtrees(default_allocator());
		if (array::size(active_groups) > 1) {
			array::resize(subtree_groups, (u32)doc.data->nodes_count);
			array::resize(state_machine_groups, (u32)doc.data->nodes_count);
			array::resize(mixed_subtrees, (u32)doc.data->nodes_count);
			for (u32 i = 0; i < array::size(subtree_groups); ++i) {
				subtree_groups[i] = UINT32_MAX;
				state_machine_groups[i] = UINT32_MAX;
				mixed_subtrees[i] = 0;
			}
			for (cgltf_size i = 0; i < active_scene->nodes_count; ++i) {
				collect_subtree_skin_groups(subtree_groups
					, mixed_subtrees
					, doc
					, attachments
					, active_scene->nodes[i]
					);
			}
			for (cgltf_size i = 0; i < active_scene->nodes_count; ++i) {
				mark_state_machine_roots(state_machine_groups
					, subtree_groups
					, mixed_subtrees
					, doc
					, active_scene->nodes[i]
					, UINT32_MAX
					);
			}
		}

		for (u32 i = 0; i < array::size(attachments); ++i) {
			RigidAttachment &attachment = attachments[i];
			cgltf_node *owner = NULL;
			if (array::size(active_groups) == 1) {
				if (active_scene->nodes_count == 1)
					owner = active_scene->nodes[0];
			} else {
				const u32 group = skin_group(doc, attachment.skin);
				owner = attachment.node;
				while (owner != NULL) {
					const u32 index = (u32)cgltf_node_index(doc.data, owner);
					if (state_machine_groups[index] == group)
						break;
					owner = owner->parent;
				}
				RETURN_IF_FALSE(MESH_GLTF, owner != NULL, opts
					, "Unable to place glTF rigid mesh '%s' under its armature"
					, node_name(doc, attachment.node)
					);
			}

			attachment.root = owner != NULL && owner->mesh != NULL ? owner->parent : owner;
			RETURN_IF_FALSE(MESH_GLTF
				, attachment.root == NULL || ancestor_or_self(attachment.root, attachment.node)
				, opts
				, "glTF rigid mesh root is not an ancestor of '%s'"
				, node_name(doc, attachment.node)
				);
		}
		return 0;
	}

	static s32 build_rigid_attachments(Array<RigidAttachment> &attachments
		, GLTFDocument &doc
		, CompileOptions &opts
		)
	{
		// The importer and compiler see the same glTF hierarchy. Rebuild the
		// attachment mapping here instead of serializing node identities in .mesh.
		Array<u32> active_groups(default_allocator());
		collect_active_skin_groups(active_groups, doc);
		s32 err = find_rigid_attachments(attachments, doc, active_groups, opts);
		ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
		if (array::empty(attachments))
			return 0;
		return resolve_rigid_attachment_roots(attachments, doc, active_groups, opts);
	}

	static bool read_float(const cgltf_accessor *accessor, cgltf_size index, f32 *out, cgltf_size count)
	{
		return accessor != NULL
			&& index < accessor->count
			&& cgltf_accessor_read_float(accessor, index, out, count)
			;
	}

	static s32 append_triangle_indices(Array<cgltf_size> &indices
		, Array<cgltf_size> &source
		, const cgltf_primitive &primitive
		, cgltf_size num_vertices
		, CompileOptions &opts
		)
	{
		array::clear(indices);
		const cgltf_size count = primitive.indices != NULL ? primitive.indices->count : num_vertices;
		RETURN_IF_FALSE(MESH_GLTF, count <= UINT32_MAX, opts
			, "glTF primitive has too many indices: %zu", count
			);
		array::reserve(source, (u32)count);
		array::resize(source, (u32)count);
		for (cgltf_size i = 0; i < count; ++i) {
			source[(u32)i] = primitive.indices != NULL
				? cgltf_accessor_read_index(primitive.indices, i)
				: i
				;
			RETURN_IF_FALSE(MESH_GLTF, source[(u32)i] < num_vertices, opts
				, "glTF primitive index %zu is out of bounds", source[(u32)i]
				);
		}

		const cgltf_size num_triangle_indices = primitive.type == cgltf_primitive_type_triangles
			? count
			: (count > 2 ? (count - 2) * 3 : 0)
			;
		RETURN_IF_FALSE(MESH_GLTF, num_triangle_indices <= UINT32_MAX, opts
			, "glTF primitive has too many triangle indices: %zu", num_triangle_indices
			);
		array::reserve(indices, (u32)num_triangle_indices);
		switch (primitive.type) {
		case cgltf_primitive_type_triangles:
			RETURN_IF_FALSE(MESH_GLTF, count % 3 == 0, opts
				, "glTF triangle primitive has an invalid index count"
				);
			for (cgltf_size i = 0; i < count; ++i)
				array::push_back(indices, source[(u32)i]);
			break;

		case cgltf_primitive_type_triangle_strip:
			for (cgltf_size i = 2; i < count; ++i) {
				if ((i & 1) == 0) {
					array::push_back(indices, source[(u32)i - 2]);
					array::push_back(indices, source[(u32)i - 1]);
				} else {
					array::push_back(indices, source[(u32)i - 1]);
					array::push_back(indices, source[(u32)i - 2]);
				}
				array::push_back(indices, source[(u32)i]);
			}
			break;

		case cgltf_primitive_type_triangle_fan:
			for (cgltf_size i = 2; i < count; ++i) {
				array::push_back(indices, source[0]);
				array::push_back(indices, source[(u32)i - 1]);
				array::push_back(indices, source[(u32)i]);
			}
			break;

		default:
			RETURN_IF_FALSE(MESH_GLTF, false, opts
				, "Unsupported glTF primitive mode %d (points and lines are not supported)"
				, primitive.type
				);
		}

		return 0;
	}

	static void append_vector3(Array<f32> &array, const Vector3 &v)
	{
		array::push_back(array, v.x);
		array::push_back(array, v.y);
		array::push_back(array, v.z);
	}

	static s32 append_skin(Geometry &g
		, const GLTFDocument &doc
		, const cgltf_skin &skin
		, const cgltf_accessor *joints
		, const cgltf_accessor *weights
		, cgltf_size vertex
		, CompileOptions &opts
		)
	{
		cgltf_uint joint_values[4] = {};
		f32 weight_values[4] = {};
		RETURN_IF_FALSE(MESH_GLTF
			, cgltf_accessor_read_uint(joints, vertex, joint_values, 4)
			&& cgltf_accessor_read_float(weights, vertex, weight_values, 4)
			, opts
			, "Failed to read glTF skin weights"
			);

		struct Influence { u16 bone; f32 weight; } influences[4];
		for (u32 i = 0; i < 4; ++i) {
			RETURN_IF_FALSE(MESH_GLTF, joint_values[i] < skin.joints_count, opts
				, "glTF joint index %u is out of bounds", joint_values[i]
				);
			influences[i].bone = bone_id(doc, skin.joints[joint_values[i]]);
			influences[i].weight = weight_values[i];
			RETURN_IF_FALSE(MESH_GLTF, influences[i].bone != UINT16_MAX, opts
				, "glTF joint is missing from the imported skeleton"
				);
		}

		for (u32 i = 0; i < 4; ++i) {
			for (u32 j = i + 1; j < 4; ++j) {
				if (influences[j].weight > influences[i].weight)
					exchange(influences[i], influences[j]);
			}
		}

		f32 total = 0.0f;
		for (u32 i = 0; i < 4; ++i)
			total += max(influences[i].weight, 0.0f);
		if (total <= FLOAT_EPSILON) {
			influences[0].weight = 1.0f;
			total = 1.0f;
		}

		for (u32 i = 0; i < 4; ++i) {
			array::push_back(g._bones, (f32)influences[i].bone);
			array::push_back(g._weights, max(influences[i].weight, 0.0f) / total);
		}
		return 0;
	}

	static void append_rigid_skin(Geometry &g, u16 bone)
	{
		array::push_back(g._bones, (f32)bone);
		array::push_back(g._bones, 0.0f);
		array::push_back(g._bones, 0.0f);
		array::push_back(g._bones, 0.0f);
		array::push_back(g._weights, 1.0f);
		array::push_back(g._weights, 0.0f);
		array::push_back(g._weights, 0.0f);
		array::push_back(g._weights, 0.0f);
	}

	static Matrix4x4 world_transform(const cgltf_node *node)
	{
		f32 matrix[16];
		cgltf_node_transform_world(node, matrix);
		return matrix4x4(matrix);
	}

	static s32 rigid_geometry_transform(Matrix4x4 &transform
		, GLTFDocument &doc
		, const RigidAttachment &attachment
		, CompileOptions &opts
		)
	{
		s32 err = select_skin(doc, attachment.skin, opts);
		ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
		RETURN_IF_FALSE(MESH_GLTF, bone_id(doc, attachment.bone) != UINT16_MAX, opts
			, "glTF rigid mesh bone '%s' is missing from skin '%s'"
			, node_name(doc, attachment.bone)
			, skin_name(doc, attachment.skin)
			);
		Matrix4x4 root_world = attachment.root != NULL
			? world_transform(attachment.root)
			: MATRIX4X4_IDENTITY
			;
		invert(root_world);
		// The imported unit flattens the path from this node up to root because
		// The mesh unit's world pose is applied after the skeleton palette.
		// Preserve those discarded transforms in the geometry, in root space.
		const Matrix4x4 relative = world_transform(attachment.node) * root_world;

		Matrix4x4 binding = MATRIX4X4_IDENTITY;
		const s32 ji = gltf::joint_index(*doc.skin, attachment.bone);
		if (ji >= 0) {
			err = gltf::binding_matrix(binding, *doc.skin, (u32)ji, opts);
			ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
		}
		Matrix4x4 bind_pose = binding * world_transform(attachment.bone);
		invert(bind_pose);
		transform = relative * bind_pose;
		return 0;
	}

	static s32 parse_primitive(Geometry &g
		, const GeometryInfo &geometry
		, const GLTFDocument &doc
		, const cgltf_primitive &primitive
		, const cgltf_skin *skin
		, const Matrix4x4 &geometry_transform
		, const Matrix4x4 &normal_transform
		, bool import_tangents
		, bool import_uvs
		, bool import_skin
		, u16 rigid_bone
		, Array<cgltf_size> &source_indices
		, Array<cgltf_size> &triangle_indices
		, Array<u32> &vertex_remap
		, Array<f32> &tangent_signs
		, CompileOptions &opts
		)
	{
		const cgltf_accessor *positions = cgltf_find_accessor(&primitive, cgltf_attribute_type_position, 0);
		const cgltf_accessor *normals = cgltf_find_accessor(&primitive, cgltf_attribute_type_normal, 0);
		const cgltf_accessor *tangents = cgltf_find_accessor(&primitive, cgltf_attribute_type_tangent, 0);
		const cgltf_accessor *uvs = cgltf_find_accessor(&primitive, cgltf_attribute_type_texcoord, 0);
		const cgltf_accessor *joints = cgltf_find_accessor(&primitive, cgltf_attribute_type_joints, 0);
		const cgltf_accessor *weights = cgltf_find_accessor(&primitive, cgltf_attribute_type_weights, 0);

		RETURN_IF_FALSE(MESH_GLTF, positions != NULL && positions->type == cgltf_type_vec3, opts
			, "glTF primitive has no valid POSITION attribute"
			);
		RETURN_IF_FALSE(MESH_GLTF
			, (joints == NULL) == (weights == NULL)
			, opts
			, "glTF primitive must provide both JOINTS_0 and WEIGHTS_0"
			);
		RETURN_IF_FALSE(MESH_GLTF
			, !import_skin
			|| rigid_bone != UINT16_MAX
			|| (joints != NULL && weights != NULL)
			, opts
			, "All primitives in a skinned glTF mesh must provide JOINTS_0 and WEIGHTS_0"
			);
		RETURN_IF_FALSE(MESH_GLTF, positions->count <= UINT32_MAX, opts
			, "glTF primitive has too many vertices: %zu", positions->count
			);
		s32 err = append_triangle_indices(triangle_indices
			, source_indices
			, primitive
			, positions->count
			, opts
			);
		ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);

		array::reserve(vertex_remap, (u32)positions->count);
		array::resize(vertex_remap, (u32)positions->count);
		for (u32 i = 0; i < array::size(vertex_remap); ++i)
			vertex_remap[i] = UINT32_MAX;
		array::clear(tangent_signs);

		const u32 position_base = (array::size(g._positions) - geometry._positions.offset) / 3;
		const u32 normal_base = (array::size(g._normals) - geometry._normals.offset) / 3;
		const u32 tangent_base = (array::size(g._tangents) - geometry._tangents.offset) / 3;
		const u32 bitangent_base = (array::size(g._bitangents) - geometry._bitangents.offset) / 3;
		const u32 bone_base = (array::size(g._bones) - geometry._bones.offset) / Geometry::MAX_BONE_WEIGHTS;
		const u32 weight_base = (array::size(g._weights) - geometry._weights.offset) / Geometry::MAX_BONE_WEIGHTS;
		const u32 uv_base = (array::size(g._uvs) - geometry._uvs.offset) / 2;

		u32 num_vertices = 0;
		for (u32 i = 0; i < array::size(triangle_indices); ++i) {
			const u32 vertex = (u32)triangle_indices[i];
			if (vertex_remap[vertex] != UINT32_MAX)
				continue;
			vertex_remap[vertex] = num_vertices++;

			f32 value[4] = {};
			RETURN_IF_FALSE(MESH_GLTF, read_float(positions, vertex, value, 3), opts
				, "Failed to read glTF positions"
				);
			append_vector3(g._positions, vector3(value) * geometry_transform);

			Vector3 normal;
			if (normals != NULL) {
				RETURN_IF_FALSE(MESH_GLTF, read_float(normals, vertex, value, 3), opts
					, "Failed to read glTF normals"
					);
				const Vector3 source_normal = vector3(value);
				Vector4 transformed = { source_normal.x, source_normal.y, source_normal.z, 0.0f };
				transformed = transformed * normal_transform;
				normal = { transformed.x, transformed.y, transformed.z };
				if (length_squared(normal) > FLOAT_EPSILON)
					normalize(normal);
				append_vector3(g._normals, normal);
			}

			if (import_tangents) {
				RETURN_IF_FALSE(MESH_GLTF, read_float(tangents, vertex, value, 4), opts
					, "Failed to read glTF tangents"
					);
				const Vector3 source_tangent = vector3(value);
				Vector4 transformed = { source_tangent.x, source_tangent.y, source_tangent.z, 0.0f };
				transformed = transformed * geometry_transform;
				Vector3 tangent = { transformed.x, transformed.y, transformed.z };
				if (length_squared(tangent) > FLOAT_EPSILON)
					normalize(tangent);
				append_vector3(g._tangents, tangent);
				if (normals != NULL)
					append_vector3(g._bitangents, cross(normal, tangent) * value[3]);
				else
					array::push_back(tangent_signs, value[3]);
			}

			if (import_uvs && uvs != NULL) {
				RETURN_IF_FALSE(MESH_GLTF, read_float(uvs, vertex, value, 2), opts
					, "Failed to read glTF texture coordinates"
					);
				array::push_back(g._uvs, value[0]);
				// Texture coordinates already use the upper-left origin.
				array::push_back(g._uvs, value[1]);
			}

			if (import_skin) {
				if (rigid_bone != UINT16_MAX) {
					append_rigid_skin(g, rigid_bone);
				} else {
					err = append_skin(g, doc, *skin, joints, weights, vertex, opts);
					ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
				}
			}
		}

		if (import_uvs && uvs == NULL && num_vertices != 0) {
			array::push_back(g._uvs, 0.0f);
			array::push_back(g._uvs, 0.0f);
		}

		for (u32 triangle = 0; triangle < array::size(triangle_indices); triangle += 3) {
			Vector3 face_normal;
			if (normals == NULL) {
				Vector3 face_positions[3];
				for (u32 corner = 0; corner < 3; ++corner) {
					const u32 vertex = (u32)triangle_indices[triangle + corner];
					const u32 index = position_base + vertex_remap[vertex];
					const u32 offset = geometry._positions.offset + index * 3;
					face_positions[corner] = { g._positions[offset + 0], g._positions[offset + 1], g._positions[offset + 2] };
				}

				face_normal = cross(face_positions[1] - face_positions[0], face_positions[2] - face_positions[0]);
				if (length_squared(face_normal) > FLOAT_EPSILON)
					normalize(face_normal);
				else
					face_normal = { 0.0f, 0.0f, 1.0f };
				append_vector3(g._normals, face_normal);
			}

			for (u32 corner = 0; corner < 3; ++corner) {
				const u32 vertex = (u32)triangle_indices[triangle + corner];
				const u32 remapped = vertex_remap[vertex];
				array::push_back(g._position_indices, position_base + remapped);
				array::push_back(g._normal_indices, normals != NULL
					? normal_base + remapped
					: normal_base + triangle / 3
					);

				if (import_tangents) {
					array::push_back(g._tangent_indices, tangent_base + remapped);
					if (normals != NULL) {
						array::push_back(g._bitangent_indices, bitangent_base + remapped);
					} else {
						const u32 tangent_offset = geometry._tangents.offset + (tangent_base + remapped) * 3;
						const Vector3 tangent = {
							g._tangents[tangent_offset + 0],
							g._tangents[tangent_offset + 1],
							g._tangents[tangent_offset + 2]
						};
						const u32 bitangent = (array::size(g._bitangents) - geometry._bitangents.offset) / 3;
						append_vector3(g._bitangents, cross(face_normal, tangent) * tangent_signs[remapped]);
						array::push_back(g._bitangent_indices, bitangent);
					}
				}

				if (import_uvs)
					array::push_back(g._uv_indices, uvs != NULL ? uv_base + remapped : uv_base);

				if (import_skin) {
					array::push_back(g._bone_indices, bone_base + remapped);
					array::push_back(g._weight_indices, weight_base + remapped);
				}
			}
		}

		return 0;
	}

	static s32 parse_geometry(Geometry &g
		, GeometryInfo &geometry
		, const GLTFDocument &doc
		, const cgltf_mesh &mesh
		, const cgltf_skin *skin
		, u16 rigid_bone
		, const Matrix4x4 &geometry_transform
		, CompileOptions &opts
		)
	{
		bool import_tangents = mesh.primitives_count != 0;
		bool import_uvs = false;
		bool import_skin = false;
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

		for (cgltf_size i = 0; i < mesh.primitives_count; ++i) {
			const cgltf_primitive &primitive = mesh.primitives[i];
			RETURN_IF_FALSE(MESH_GLTF
				, primitive.type == cgltf_primitive_type_triangles
				|| primitive.type == cgltf_primitive_type_triangle_strip
				|| primitive.type == cgltf_primitive_type_triangle_fan
				, opts
				, "Unsupported glTF primitive mode %d (points and lines are not supported)"
				, primitive.type
				);
			RETURN_IF_FALSE(MESH_GLTF
				, !primitive.has_draco_mesh_compression || primitive.attributes_count != 0
				, opts
				, "Draco-compressed glTF primitive has no uncompressed fallback"
				);
			if (primitive.targets_count != 0)
				opts.warning(MESH_GLTF, "glTF morph targets are not supported and will be ignored");

			const cgltf_accessor *joints = cgltf_find_accessor(&primitive, cgltf_attribute_type_joints, 0);
			const cgltf_accessor *weights = cgltf_find_accessor(&primitive, cgltf_attribute_type_weights, 0);
			import_tangents = import_tangents && cgltf_find_accessor(&primitive, cgltf_attribute_type_tangent, 0) != NULL;
			import_uvs = import_uvs || cgltf_find_accessor(&primitive, cgltf_attribute_type_texcoord, 0) != NULL;
			import_skin = import_skin || joints != NULL || weights != NULL;
		}

		RETURN_IF_FALSE(MESH_GLTF, !import_skin || skin != NULL || rigid_bone != UINT16_MAX, opts
			, "Skinned glTF mesh node has no skin"
			);
		import_skin = import_skin || rigid_bone != UINT16_MAX;

		Matrix4x4 normal_transform = geometry_transform;
		invert(normal_transform);
		transpose(normal_transform);

		Array<cgltf_size> source_indices(default_allocator());
		Array<cgltf_size> triangle_indices(default_allocator());
		Array<u32> vertex_remap(default_allocator());
		Array<f32> tangent_signs(default_allocator());
		for (cgltf_size i = 0; i < mesh.primitives_count; ++i) {
			const cgltf_primitive &primitive = mesh.primitives[i];
			char suffix[32];
			stbsp_snprintf(suffix, sizeof(suffix), "_%u", (u32)i);
			DynamicString slot(ta);
			if (primitive.material != NULL)
				slot = material_name(doc, primitive.material);
			else
				slot = "default";
			while (hash_map::has(used_slots, slot))
				slot += suffix;
			hash_map::set(used_slots, slot, true);

			const u32 index_offset = array::size(g._position_indices) - geometry._position_indices.offset;
			s32 err = parse_primitive(g
				, geometry
				, doc
				, primitive
				, skin
				, geometry_transform
				, normal_transform
				, import_tangents
				, import_uvs
				, import_skin
				, rigid_bone
				, source_indices
				, triangle_indices
				, vertex_remap
				, tangent_signs
				, opts
				);
			ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
			const u32 num_indices = array::size(g._position_indices)
				- geometry._position_indices.offset
				- index_offset
				;
			if (num_indices != 0)
				array::push_back(g._material_ranges, { slot.to_string_id(), index_offset, num_indices });
		}
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

	s32 parse(Mesh &m
		, const char *path
		, const MeshImportOptions &import_options
		, CompileOptions &opts
		)
	{
		GLTFDocument doc(default_allocator());
		s32 err = gltf::parse(doc, path, opts);
		ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
		Array<RigidAttachment> rigid_attachments(default_allocator());
		if (import_options.rigid_skinning) {
			err = build_rigid_attachments(rigid_attachments, doc, opts);
			ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
		}

		for (cgltf_size i = 0; i < doc.data->nodes_count; ++i) {
			cgltf_node *source = &doc.data->nodes[i];
			if (!node_in_scene(doc, source))
				continue;

			Node node(default_allocator());
			node._local_pose = local_transform(source);

			DynamicString name(default_allocator());
			name = node_name(doc, source);
			if (source->mesh != NULL) {
				const RigidAttachment *rigid = rigid_attachment(rigid_attachments, source);
				Matrix4x4 geometry_transform = MATRIX4X4_IDENTITY;
				if (source->skin != NULL) {
					err = select_skin(doc, source->skin, opts);
					ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
					geometry_transform = skin_transform(doc, source->skin);
				} else if (rigid != NULL) {
					err = rigid_geometry_transform(geometry_transform, doc, *rigid, opts);
					ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
				}

				GeometryInfo geometry = {};
				const u16 rigid_bone = rigid != NULL ? bone_id(doc, rigid->bone) : UINT16_MAX;
				RETURN_IF_FALSE(MESH_GLTF, rigid == NULL || rigid_bone != 0, opts
					, "Rigid glTF mesh '%s' cannot attach to skeleton root '%s'"
					, node_name(doc, source)
					, node_name(doc, rigid->bone)
					);
				err = parse_geometry(m._geometry
					, geometry
					, doc
					, *source->mesh
					, source->skin
					, rigid_bone
					, geometry_transform
					, opts
					);
				ENSURE_OR_RETURN(MESH_GLTF, err == 0, opts);
				if (geometry._position_indices.count != 0) {
					node._geometry = name;
					hash_map::set(m._geometries, name, geometry);
				}
			}
			hash_map::set(m._nodes, name, node);
		}

		return 0;
	}

} // namespace gltf

} // namespace crown

#endif // if CROWN_CAN_COMPILE
