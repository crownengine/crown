/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/gltf_document.h"

#if CROWN_CAN_COMPILE
#   include "core/math/math.inl"
#   include "core/containers/array.inl"
#   include "core/containers/hash_map.inl"
#   include "core/containers/vector.inl"
#   include "core/filesystem/path.h"
#   include "core/math/matrix4x4.inl"
#   include "core/memory/allocator.h"
#   include "core/memory/globals.h"
#   include "core/strings/dynamic_string.inl"
#   include "core/strings/string_id.inl"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/mesh_skeleton_resource.h"
#   include <cgltf.h>
#   include <stb_sprintf.h>
#   include <stdint.h> // SIZE_MAX
#   include <string.h> // memcpy, memset, strchr, strcmp

LOG_SYSTEM(GLTF_DOCUMENT, "gltf_document")

namespace crown
{
namespace gltf
{
	static void *memory_allocate(void *, cgltf_size size)
	{
		if (size > UINT32_MAX)
			return NULL;
		return default_allocator().allocate(size != 0 ? (u32)size : 1u);
	}

	static void memory_deallocate(void *, void *data)
	{
		default_allocator().deallocate(data);
	}

	static const char *result_name(cgltf_result result)
	{
		switch (result) {
		case cgltf_result_success:         return "success";
		case cgltf_result_data_too_short:  return "data too short";
		case cgltf_result_unknown_format:  return "unknown format";
		case cgltf_result_invalid_json:    return "invalid JSON";
		case cgltf_result_invalid_gltf:    return "invalid glTF";
		case cgltf_result_invalid_options: return "invalid options";
		case cgltf_result_file_not_found:  return "file not found";
		case cgltf_result_io_error:        return "I/O error";
		case cgltf_result_out_of_memory:   return "out of memory";
		case cgltf_result_legacy_gltf:     return "legacy glTF";
		default:                           return "unknown error";
		}
	}

	static cgltf_result file_read(const cgltf_memory_options *memory_options
		, const cgltf_file_options *file_options
		, const char *path
		, cgltf_size *size
		, void **data
		)
	{
		CompileOptions &opts = *(CompileOptions *)file_options->user_data;
		if (strchr(path, ':') != NULL
			|| !path::is_within_root(path)
			) {
			return cgltf_result_io_error;
		}

		if (!opts.file_exists(path))
			return cgltf_result_file_not_found;

		Buffer buf = opts.read(path);
		const cgltf_size expected_size = *size;
		if (expected_size != 0 && array::size(buf) < expected_size)
			return cgltf_result_data_too_short;

		const cgltf_size read_size = expected_size != 0 ? expected_size : array::size(buf);
		void *mem = memory_options->alloc_func(memory_options->user_data, read_size);
		if (mem == NULL)
			return cgltf_result_out_of_memory;
		if (read_size != 0)
			memcpy(mem, array::begin(buf), read_size);

		*size = read_size;
		*data = mem;
		return cgltf_result_success;
	}

	static void file_release(const cgltf_memory_options *memory_options
		, const cgltf_file_options *
		, void *data
		)
	{
		memory_options->free_func(memory_options->user_data, data);
	}

	static bool range_valid(cgltf_size offset, cgltf_size size, cgltf_size total)
	{
		return offset <= total && size <= total - offset;
	}

	static bool strided_range_valid(cgltf_size offset
		, cgltf_size count
		, cgltf_size stride
		, cgltf_size element_size
		, cgltf_size total
		)
	{
		if (offset > total)
			return false;
		if (count == 0)
			return true;
		if (stride < element_size || count - 1 > (SIZE_MAX - element_size) / stride)
			return false;
		return range_valid(offset, (count - 1)*stride + element_size, total);
	}

	static s32 validate_ranges(cgltf_data *data, CompileOptions &opts)
	{
		for (cgltf_size i = 0; i < data->buffer_views_count; ++i) {
			const cgltf_buffer_view &view = data->buffer_views[i];
			RETURN_IF_FALSE(GLTF_DOCUMENT, view.buffer != NULL, opts
				, "glTF buffer view %zu has no buffer", i
				);
			RETURN_IF_FALSE(GLTF_DOCUMENT, range_valid(view.offset, view.size, view.buffer->size), opts
				, "glTF buffer view %zu is out of bounds", i
				);
		}

		for (cgltf_size i = 0; i < data->accessors_count; ++i) {
			const cgltf_accessor &accessor = data->accessors[i];
			const cgltf_size element_size = cgltf_calc_size(accessor.type, accessor.component_type);
			RETURN_IF_FALSE(GLTF_DOCUMENT, element_size != 0, opts
				, "glTF accessor %zu has an invalid element type", i
				);

			if (accessor.buffer_view != NULL) {
				const cgltf_size stride = accessor.stride != 0 ? accessor.stride : element_size;
				RETURN_IF_FALSE(GLTF_DOCUMENT
					, strided_range_valid(accessor.offset, accessor.count, stride, element_size, accessor.buffer_view->size)
					, opts
					, "glTF accessor %zu is out of bounds"
					, i
					);
			}

			if (accessor.is_sparse) {
				RETURN_IF_FALSE(GLTF_DOCUMENT, accessor.sparse.count <= accessor.count, opts
					, "glTF sparse accessor %zu has too many entries", i
					);
				RETURN_IF_FALSE(GLTF_DOCUMENT
					, accessor.sparse.indices_buffer_view != NULL && accessor.sparse.values_buffer_view != NULL
					, opts
					, "glTF sparse accessor %zu has missing buffer views"
					, i
					);

				const cgltf_size index_size = cgltf_component_size(accessor.sparse.indices_component_type);
				RETURN_IF_FALSE(GLTF_DOCUMENT, index_size != 0, opts
					, "glTF sparse accessor %zu has an invalid index type", i
					);
				RETURN_IF_FALSE(GLTF_DOCUMENT
					, strided_range_valid(accessor.sparse.indices_byte_offset, accessor.sparse.count, index_size, index_size, accessor.sparse.indices_buffer_view->size)
					, opts
					, "glTF sparse accessor %zu indices are out of bounds"
					, i
					);
				RETURN_IF_FALSE(GLTF_DOCUMENT
					, strided_range_valid(accessor.sparse.values_byte_offset, accessor.sparse.count, element_size, element_size, accessor.sparse.values_buffer_view->size)
					, opts
					, "glTF sparse accessor %zu values are out of bounds"
					, i
					);
			}
		}

		return 0;
	}

	static void make_names(Vector<DynamicString> &names
		, const char *kind
		, cgltf_size count
		, const char *(*raw_name)(cgltf_size, void *)
		, void *objects
		)
	{
		HashMap<DynamicString, u8> used(default_allocator());
		for (cgltf_size i = 0; i < count; ++i) {
			DynamicString name(default_allocator());
			const char *raw = raw_name(i, objects);
			if (raw != NULL && raw[0] != '\0') {
				name = raw;
			} else {
				char buf[64];
				stbsp_snprintf(buf, sizeof(buf), "%s_%zu", kind, i);
				name = buf;
			}

			if (hash_map::has(used, name)) {
				char suffix[64];
				stbsp_snprintf(suffix, sizeof(suffix), "_%zu", i);
				name += suffix;
				while (hash_map::has(used, name))
					name += suffix;
			}
			hash_map::set(used, name, (u8)1);
			vector::push_back(names, name);
		}
	}

	static const char *raw_node_name(cgltf_size i, void *p)
	{
		return ((cgltf_node *)p)[i].name;
	}

	static const char *raw_material_name(cgltf_size i, void *p)
	{
		return ((cgltf_material *)p)[i].name;
	}

	static const char *raw_skin_name(cgltf_size i, void *p)
	{
		return ((cgltf_skin *)p)[i].name;
	}

	static const char *raw_animation_name(cgltf_size i, void *p)
	{
		return ((cgltf_animation *)p)[i].name;
	}

	const cgltf_scene *scene(const GLTFDocument &gltf)
	{
		return gltf.data->scene != NULL
			? gltf.data->scene
			: (gltf.data->scenes_count != 0 ? &gltf.data->scenes[0] : NULL)
			;
	}

	bool node_in_scene(const GLTFDocument &gltf, const cgltf_node *node)
	{
		const cgltf_scene *active_scene = scene(gltf);
		if (active_scene == NULL)
			return false;

		while (node->parent != NULL)
			node = node->parent;
		for (cgltf_size i = 0; i < active_scene->nodes_count; ++i) {
			if (active_scene->nodes[i] == node)
				return true;
		}
		return false;
	}

	s32 joint_index(const cgltf_skin &skin, const cgltf_node *joint)
	{
		for (cgltf_size i = 0; i < skin.joints_count; ++i) {
			if (skin.joints[i] == joint)
				return (s32)i;
		}
		return -1;
	}

	static bool same_joints(const cgltf_skin &a, const cgltf_skin &b)
	{
		if (a.joints_count != b.joints_count)
			return false;
		for (cgltf_size i = 0; i < a.joints_count; ++i) {
			if (joint_index(b, a.joints[i]) < 0)
				return false;
		}
		return true;
	}

	s32 binding_matrix(Matrix4x4 &matrix
		, const cgltf_skin &skin
		, u32 joint
		, CompileOptions &opts
		)
	{
		matrix = MATRIX4X4_IDENTITY;
		if (skin.inverse_bind_matrices == NULL)
			return 0;

		f32 values[16];
		RETURN_IF_FALSE(GLTF_DOCUMENT
			, cgltf_accessor_read_float(skin.inverse_bind_matrices, (cgltf_size)joint, values, 16)
			, opts
			, "Failed to read glTF inverse bind matrix"
			);
		matrix = matrix4x4(values);
		return 0;
	}

	static bool matrix_near(const Matrix4x4 &a, const Matrix4x4 &b)
	{
		const f32 *av = (const f32 *)&a;
		const f32 *bv = (const f32 *)&b;
		for (u32 i = 0; i < 16; ++i) {
			if (fabs(av[i] - bv[i]) > 1e-3f)
				return false;
		}
		return true;
	}

	static s32 skin_correction(Matrix4x4 &correction
		, const cgltf_skin &canonical
		, const cgltf_skin &skin
		, CompileOptions &opts
		)
	{
		correction = MATRIX4X4_IDENTITY;
		if (skin.joints_count == 0)
			return 0;

		const s32 canonical_joint = joint_index(canonical, skin.joints[0]);
		RETURN_IF_FALSE(GLTF_DOCUMENT, canonical_joint >= 0, opts
			, "glTF skins do not reference the same joints"
			);

		Matrix4x4 canonical_binding;
		Matrix4x4 source_binding;
		s32 err = binding_matrix(canonical_binding, canonical, (u32)canonical_joint, opts);
		ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
		err = binding_matrix(source_binding, skin, 0, opts);
		ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
		Matrix4x4 inverse_canonical = canonical_binding;
		invert(inverse_canonical);
		correction = source_binding * inverse_canonical;

		for (cgltf_size i = 1; i < skin.joints_count; ++i) {
			const s32 ci = joint_index(canonical, skin.joints[i]);
			RETURN_IF_FALSE(GLTF_DOCUMENT, ci >= 0, opts
				, "glTF skins do not reference the same joints"
				);
			err = binding_matrix(canonical_binding, canonical, (u32)ci, opts);
			ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
			err = binding_matrix(source_binding, skin, i, opts);
			ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
			inverse_canonical = canonical_binding;
			invert(inverse_canonical);
			const Matrix4x4 candidate = source_binding * inverse_canonical;
			RETURN_IF_FALSE(GLTF_DOCUMENT, matrix_near(candidate, correction), opts
				, "glTF skins sharing one skeleton have incompatible bind matrices"
				);
		}
		return 0;
	}

	static s32 build_skin_groups(GLTFDocument &gltf, CompileOptions &opts)
	{
		array::resize(gltf.skin_groups, (u32)gltf.data->skins_count);
		array::resize(gltf.skin_transforms, (u32)gltf.data->skins_count);
		for (u32 i = 0; i < array::size(gltf.skin_groups); ++i) {
			gltf.skin_groups[i] = UINT32_MAX;
			gltf.skin_transforms[i] = MATRIX4X4_IDENTITY;
		}

		for (cgltf_size i = 0; i < gltf.data->skins_count; ++i) {
			if (gltf.skin_groups[(u32)i] != UINT32_MAX)
				continue;
			const u32 group = array::size(gltf.skin_group_primaries);
			array::push_back(gltf.skin_group_primaries, (u32)i);
			gltf.skin_groups[(u32)i] = group;

			for (cgltf_size j = i + 1; j < gltf.data->skins_count; ++j) {
				if (gltf.skin_groups[(u32)j] != UINT32_MAX
					|| !same_joints(gltf.data->skins[i], gltf.data->skins[j])
					)
					continue;

				Matrix4x4 correction;
				s32 err = skin_correction(correction, gltf.data->skins[i], gltf.data->skins[j], opts);
				ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
				gltf.skin_groups[(u32)j] = group;
				gltf.skin_transforms[(u32)j] = correction;
			}
		}
		return 0;
	}

	static cgltf_skin *default_skin(GLTFDocument &gltf)
	{
		for (cgltf_size i = 0; i < gltf.data->nodes_count; ++i) {
			cgltf_node *node = &gltf.data->nodes[i];
			if (node_in_scene(gltf, node) && node->skin != NULL)
				return node->skin;
		}

		return gltf.data->skins_count != 0 ? &gltf.data->skins[0] : NULL;
	}

	static bool marked_node(const GLTFDocument &gltf, const cgltf_node *node, const Array<u8> &marked)
	{
		return marked[cgltf_node_index(gltf.data, node)] != 0;
	}

	static s32 add_bone(GLTFDocument &gltf, cgltf_node *node, const Array<u8> &marked, CompileOptions &opts)
	{
		RETURN_IF_FALSE(GLTF_DOCUMENT, vector::size(gltf.bones) < MESH_SKELETON_MAX_BONES, opts
			, "Maximum number of bones reached %u", MESH_SKELETON_MAX_BONES
			);
		u16 id = (u16)vector::size(gltf.bones);
		vector::push_back(gltf.bones, node);
		hash_map::set(gltf.bone_ids, StringId32(node_name(gltf, node)), id);

		for (cgltf_size i = 0; i < node->children_count; ++i) {
			if (marked_node(gltf, node->children[i], marked)) {
				s32 err = add_bone(gltf, node->children[i], marked, opts);
				ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
			}
		}
		return 0;
	}

	static s32 build_bones(GLTFDocument &gltf, CompileOptions &opts)
	{
		Array<u8> marked(default_allocator());
		array::resize(marked, (u32)gltf.data->nodes_count);
		memset(array::begin(marked), 0, array::size(marked));

		if (gltf.skin != NULL) {
			for (cgltf_size i = 0; i < gltf.skin->joints_count; ++i) {
				cgltf_node *node = gltf.skin->joints[i];
				while (node != NULL) {
					marked[cgltf_node_index(gltf.data, node)] = 1;
					node = node->parent;
				}
			}
		} else if (gltf.data->animations_count != 0) {
			const cgltf_animation &animation = gltf.data->animations[0];
			for (cgltf_size i = 0; i < animation.channels_count; ++i) {
				const cgltf_animation_channel &channel = animation.channels[i];
				if (channel.target_node == NULL
					|| (channel.target_path != cgltf_animation_path_type_translation
					&& channel.target_path != cgltf_animation_path_type_rotation)
					)
					continue;
				cgltf_node *node = channel.target_node;
				while (node != NULL) {
					marked[cgltf_node_index(gltf.data, node)] = 1;
					node = node->parent;
				}
			}
		}

		// RenderWorld reserves palette slot zero for the mesh model transform.
		// Keep a top-level joint out of that slot because glTF vertices may be
		// weighted directly to it.
		if (gltf.skin != NULL) {
			for (cgltf_size i = 0; i < gltf.data->nodes_count; ++i) {
				cgltf_node *node = &gltf.data->nodes[i];
				if (!marked[i] || (node->parent != NULL && marked_node(gltf, node->parent, marked)))
					continue;
				if (joint_index(*gltf.skin, node) >= 0)
					vector::push_back(gltf.bones, (cgltf_node *)NULL);
				break;
			}
		}

		for (cgltf_size i = 0; i < gltf.data->nodes_count; ++i) {
			cgltf_node *node = &gltf.data->nodes[i];
			if (marked[i] && (node->parent == NULL || !marked_node(gltf, node->parent, marked))) {
				s32 err = add_bone(gltf, node, marked, opts);
				ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
			}
		}

		return 0;
	}

	s32 select_skin(GLTFDocument &gltf, const cgltf_skin *skin, CompileOptions &opts)
	{
		vector::clear(gltf.bones);
		hash_map::clear(gltf.bone_ids);
		gltf.skin = NULL;

		if (skin != NULL) {
			const u32 group = skin_group(gltf, skin);
			RETURN_IF_FALSE(GLTF_DOCUMENT, group != UINT32_MAX, opts
				, "glTF skin is not part of the document"
				);
			gltf.skin = &gltf.data->skins[gltf.skin_group_primaries[group]];
		}
		return build_bones(gltf, opts);
	}

	s32 select_skin(GLTFDocument &gltf, const char *name, CompileOptions &opts)
	{
		if (name == NULL || name[0] == '\0')
			return select_skin(gltf, default_skin(gltf), opts);
		for (cgltf_size i = 0; i < gltf.data->skins_count; ++i) {
			if (strcmp(name, gltf.skin_names[(u32)i].c_str()) == 0)
				return select_skin(gltf, &gltf.data->skins[i], opts);
		}
		RETURN_IF_FALSE(GLTF_DOCUMENT, false, opts
			, "No matching glTF skin '%s'", name
			);
	}

	s32 parse(GLTFDocument &gltf, const char *path, CompileOptions &opts)
	{
		cgltf_options options = {};
		options.memory.alloc_func = memory_allocate;
		options.memory.free_func = memory_deallocate;
		options.file.read = file_read;
		options.file.release = file_release;
		options.file.user_data = &opts;

		cgltf_result result = cgltf_parse_file(&options, path, &gltf.data);
		RETURN_IF_FALSE(GLTF_DOCUMENT, result == cgltf_result_success, opts
			, "cgltf: %s", result_name(result)
			);

		for (cgltf_size i = 0; i < gltf.data->extensions_required_count; ++i) {
			const char *extension = gltf.data->extensions_required[i];
			RETURN_IF_FALSE(GLTF_DOCUMENT
				, strcmp(extension, "KHR_draco_mesh_compression") != 0
				&& strcmp(extension, "EXT_meshopt_compression") != 0
				&& strcmp(extension, "KHR_texture_basisu") != 0
				, opts
				, "Unsupported required glTF extension '%s'", extension
				);
		}

		result = cgltf_load_buffers(&options, gltf.data, path);
		RETURN_IF_FALSE(GLTF_DOCUMENT, result == cgltf_result_success, opts
			, "cgltf: failed to load buffers: %s", result_name(result)
			);

		s32 err = validate_ranges(gltf.data, opts);
		ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
		result = cgltf_validate(gltf.data);
		RETURN_IF_FALSE(GLTF_DOCUMENT, result == cgltf_result_success, opts
			, "cgltf: validation failed: %s", result_name(result)
			);
		RETURN_IF_FALSE(GLTF_DOCUMENT, scene(gltf) != NULL, opts, "glTF contains no scene");

		make_names(gltf.node_names, "node", gltf.data->nodes_count, raw_node_name, gltf.data->nodes);
		make_names(gltf.material_names, "material", gltf.data->materials_count, raw_material_name, gltf.data->materials);
		make_names(gltf.skin_names, "skin", gltf.data->skins_count, raw_skin_name, gltf.data->skins);
		make_names(gltf.animation_names, "animation", gltf.data->animations_count, raw_animation_name, gltf.data->animations);

		err = build_skin_groups(gltf, opts);
		ENSURE_OR_RETURN(GLTF_DOCUMENT, err == 0, opts);
		return select_skin(gltf, default_skin(gltf), opts);
	}

	const char *node_name(const GLTFDocument &gltf, const cgltf_node *node)
	{
		return gltf.node_names[cgltf_node_index(gltf.data, node)].c_str();
	}

	const char *material_name(const GLTFDocument &gltf, const cgltf_material *material)
	{
		return gltf.material_names[cgltf_material_index(gltf.data, material)].c_str();
	}

	const char *skin_name(const GLTFDocument &gltf, const cgltf_skin *skin)
	{
		return gltf.skin_names[cgltf_skin_index(gltf.data, skin)].c_str();
	}

	const char *animation_name(const GLTFDocument &gltf, const cgltf_animation *animation)
	{
		return gltf.animation_names[cgltf_animation_index(gltf.data, animation)].c_str();
	}

	u16 bone_id(const GLTFDocument &gltf, const cgltf_node *node)
	{
		u16 not_found = UINT16_MAX;
		return hash_map::get(gltf.bone_ids, StringId32(node_name(gltf, node)), not_found);
	}

	u32 skin_group(const GLTFDocument &gltf, const cgltf_skin *skin)
	{
		if (skin == NULL)
			return UINT32_MAX;
		const cgltf_size index = cgltf_skin_index(gltf.data, skin);
		return index < array::size(gltf.skin_groups)
			? gltf.skin_groups[(u32)index]
			: UINT32_MAX
			;
	}

	const Matrix4x4 &skin_transform(const GLTFDocument &gltf, const cgltf_skin *skin)
	{
		CE_ENSURE(skin != NULL);
		return gltf.skin_transforms[(u32)cgltf_skin_index(gltf.data, skin)];
	}

	Matrix4x4 skeleton_root_transform(const GLTFDocument &gltf)
	{
		if (gltf.skin == NULL)
			return MATRIX4X4_IDENTITY;

		const u32 selected_group = skin_group(gltf, gltf.skin);
		for (cgltf_size i = 0; i < gltf.data->nodes_count; ++i) {
			const cgltf_node *node = &gltf.data->nodes[i];
			if (node->mesh == NULL
				|| node->skin == NULL
				|| skin_group(gltf, node->skin) != selected_group
				|| !node_in_scene(gltf, node)
				)
				continue;

			// RenderWorld resets a skinned mesh node's local pose and uses its
			// parent world transform as u_model[0]. Evaluate the skeleton relative
			// to that model transform so it is not applied again by the skin palette.
			Matrix4x4 model = MATRIX4X4_IDENTITY;
			if (node->parent != NULL) {
				f32 matrix[16];
				cgltf_node_transform_world(node->parent, matrix);
				model = matrix4x4(matrix);
			}
			invert(model);
			return model;
		}

		return MATRIX4X4_IDENTITY;
	}

	Vector3 vector3(const f32 *v)
	{
		return { v[0], -v[2], v[1] };
	}

	Quaternion quaternion(const f32 *q)
	{
		return { q[0], -q[2], q[1], q[3] };
	}

	Matrix4x4 matrix4x4(const f32 *m)
	{
		Matrix4x4 out;
		out.x = {  m[ 0], -m[ 2],  m[ 1], 0.0f };
		out.y = { -m[ 8],  m[10], -m[ 9], 0.0f };
		out.z = {  m[ 4], -m[ 6],  m[ 5], 0.0f };
		out.t = {  m[12], -m[14],  m[13], 1.0f };
		return out;
	}

	Matrix4x4 local_transform(const cgltf_node *node)
	{
		f32 m[16];
		cgltf_node_transform_local(node, m);
		return matrix4x4(m);
	}

} // namespace gltf

GLTFDocument::GLTFDocument(Allocator &a)
	: data(NULL)
	, skin(NULL)
	, node_names(a)
	, material_names(a)
	, skin_names(a)
	, animation_names(a)
	, skin_groups(a)
	, skin_group_primaries(a)
	, skin_transforms(a)
	, bones(a)
	, bone_ids(a)
{
}

GLTFDocument::~GLTFDocument()
{
	cgltf_free(data);
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
