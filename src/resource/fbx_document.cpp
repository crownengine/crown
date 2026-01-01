/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/fbx_document.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/containers/hash_map.inl"
#   include "core/strings/string_id.inl"
#   include "resource/compile_options.inl"
#   include <ufbx.h>

namespace crown
{
namespace fbx
{
	static ufbx_node *find_first_non_bone_parent(ufbx_node *bone_node)
	{
		while (bone_node && bone_node->bone != NULL)
			bone_node = bone_node->parent;

		return bone_node;
	}

	static ufbx_node *find_skeleton_root(ufbx_node *node)
	{
		if (node->bone != NULL)
			return node;

		for (size_t i = 0; i < node->children.count; ++i) {
			ufbx_node *n = find_skeleton_root(node->children.data[i]);
			if (n != NULL)
				return find_first_non_bone_parent(n);
		}

		return NULL;
	}

	u16 bone_id(const FBXDocument &fbx, StringId32 bone_name)
	{
		u16 deffault_bone_id = UINT16_MAX;
		return hash_map::get(fbx.bone_ids, bone_name, deffault_bone_id);
	}

	u16 bone_id(const FBXDocument &fbx, const char *bone_name)
	{
		return bone_id(fbx, StringId32(bone_name));
	}

	static s32 populate_bone_ids(FBXDocument &fbx, ufbx_node *bone, CompileOptions &opts, u32 *debug_num_bones)
	{
		u16 bone_id = (u16)hash_map::size(fbx.bone_ids);
		StringId32 bone_name = StringId32(bone->name.data);
		hash_map::set(fbx.bone_ids, bone_name, bone_id);
		(*debug_num_bones)++;

		for (size_t i = 0; i < bone->children.count; ++i) {
			ufbx_node *child = bone->children.data[i];
			s32 err = populate_bone_ids(fbx, child, opts, debug_num_bones);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		return 0;
	}

	s32 parse(FBXDocument &fbx, Buffer &buf, CompileOptions &opts)
	{
		// Keep in sync with mesh_resource_fbx.vala!
		ufbx_load_opts load_opts = {};
		load_opts.target_camera_axes =
		{
			UFBX_COORDINATE_AXIS_POSITIVE_X,
			UFBX_COORDINATE_AXIS_POSITIVE_Z,
			UFBX_COORDINATE_AXIS_NEGATIVE_Y
		};
		load_opts.target_light_axes =
		{
			UFBX_COORDINATE_AXIS_POSITIVE_X,
			UFBX_COORDINATE_AXIS_POSITIVE_Y,
			UFBX_COORDINATE_AXIS_POSITIVE_Z
		};
		load_opts.target_axes = ufbx_axes_right_handed_z_up;
		load_opts.target_unit_meters = 1.0f;
		load_opts.space_conversion = UFBX_SPACE_CONVERSION_TRANSFORM_ROOT;

		ufbx_error error;
		fbx.scene = ufbx_load_memory(array::begin(buf)
			, array::size(buf)
			, &load_opts
			, &error
			);
		RETURN_IF_FALSE(fbx.scene != NULL
			, opts
			, "ufbx: %s"
			, error.description.data
			);

		fbx.skeleton_root_node = find_skeleton_root(fbx.scene->root_node);
		if (fbx.skeleton_root_node != NULL) {
			u32 debug_num_bones = 0;
			s32 err = populate_bone_ids(fbx, fbx.skeleton_root_node, opts, &debug_num_bones);
			ENSURE_OR_RETURN(err == 0, opts);
			RETURN_IF_FALSE(debug_num_bones == hash_map::size(fbx.bone_ids)
				, opts
				, "Bone mismatch expected/actual %u/%u"
				, debug_num_bones
				, hash_map::size(fbx.bone_ids)
				);
		}

		return 0;
	}

	///
	s32 parse(FBXDocument &fbx, const char *path, CompileOptions &opts)
	{
		RETURN_IF_FILE_MISSING(path, opts);
		Buffer buf = opts.read(path);
		return parse(fbx, buf, opts);
	}

} // namespace fbx

FBXDocument::FBXDocument(Allocator &a)
	: scene(NULL)
	, skeleton_root_node(NULL)
	, bone_ids(a)
{
}

FBXDocument::~FBXDocument()
{
	ufbx_free_scene(scene);
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
