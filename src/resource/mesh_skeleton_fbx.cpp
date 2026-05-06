/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_skeleton_fbx.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/containers/hash_map.inl"
#   include "core/math/matrix4x4.inl"
#   include "core/math/vector2.inl"
#   include "core/math/vector3.inl"
#   include "core/memory/temp_allocator.inl"
#   include "core/strings/dynamic_string.inl"
#   include "core/strings/string_id.inl"
#   include "resource/mesh_skeleton.h"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/fbx_document.h"
#   include <ufbx.h>

LOG_SYSTEM(MESH_SKELETON_FBX, "mesh_skeleton_fbx")

namespace crown
{
namespace fbx
{
	static ufbx_skin_cluster *find_cluster(ufbx_scene *scene, ufbx_node *node)
	{
		for (size_t i = 0; i < scene->skin_clusters.count; ++i) {
			ufbx_skin_cluster *cluster = scene->skin_clusters.data[i];
			if (cluster->bone_node == node)
				return cluster;
		}

		return NULL;
	}

	static s32 parse_skeleton(AnimationSkeleton &as, FBXDocument &fbx, ufbx_node *bone, CompileOptions &opts)
	{
		BoneTransform bone_tm;
		u16 parent_bone_id;

		if (bone->is_root) {
			bone_tm.position = VECTOR3_ZERO;
			bone_tm.rotation = QUATERNION_IDENTITY;
			bone_tm.scale = VECTOR3_ONE;

			parent_bone_id = UINT16_MAX;
		} else {
			bone_tm.position.x = (f32)bone->local_transform.translation.x;
			bone_tm.position.y = (f32)bone->local_transform.translation.y;
			bone_tm.position.z = (f32)bone->local_transform.translation.z;
			bone_tm.rotation.x = (f32)bone->local_transform.rotation.x;
			bone_tm.rotation.y = (f32)bone->local_transform.rotation.y;
			bone_tm.rotation.z = (f32)bone->local_transform.rotation.z;
			bone_tm.rotation.w = (f32)bone->local_transform.rotation.w;
			bone_tm.scale.x = (f32)bone->local_transform.scale.x;
			bone_tm.scale.y = (f32)bone->local_transform.scale.y;
			bone_tm.scale.z = (f32)bone->local_transform.scale.z;

			parent_bone_id = bone_id(fbx, bone->parent->name.data);
		}

		array::push_back(as.local_transforms, bone_tm);
		array::push_back(as.parents, (u32)parent_bone_id);

		ufbx_skin_cluster *cluster = find_cluster(fbx.scene, bone);
		if (cluster != NULL) {
			Matrix4x4 m;
			m.x.x = (f32)cluster->geometry_to_bone.cols[0].x;
			m.x.y = (f32)cluster->geometry_to_bone.cols[0].y;
			m.x.z = (f32)cluster->geometry_to_bone.cols[0].z;
			m.x.w = 0.0f;
			m.y.x = (f32)cluster->geometry_to_bone.cols[1].x;
			m.y.y = (f32)cluster->geometry_to_bone.cols[1].y;
			m.y.z = (f32)cluster->geometry_to_bone.cols[1].z;
			m.y.w = 0.0f;
			m.z.x = (f32)cluster->geometry_to_bone.cols[2].x;
			m.z.y = (f32)cluster->geometry_to_bone.cols[2].y;
			m.z.z = (f32)cluster->geometry_to_bone.cols[2].z;
			m.z.w = 0.0f;
			m.t.x = (f32)cluster->geometry_to_bone.cols[3].x;
			m.t.y = (f32)cluster->geometry_to_bone.cols[3].y;
			m.t.z = (f32)cluster->geometry_to_bone.cols[3].z;
			m.t.w = 1.0f;
			array::push_back(as.binding_matrices, m);
		} else {
			array::push_back(as.binding_matrices, MATRIX4X4_IDENTITY);
		}

		CE_ENSURE(bone_id(fbx, bone->name.data) == array::size(as.binding_matrices) - 1);

		for (size_t i = 0; i < bone->children.count; ++i) {
			ufbx_node *child = bone->children.data[i];
			s32 err = parse_skeleton(as, fbx, child, opts);
			ENSURE_OR_RETURN(MESH_SKELETON_FBX, err == 0, opts);
		}

		return 0;
	}

	s32 parse(AnimationSkeleton &as, Buffer &buf, CompileOptions &opts)
	{
		FBXDocument fbx(default_allocator());
		s32 err = fbx::parse(fbx, buf, opts);
		ENSURE_OR_RETURN(MESH_SKELETON_FBX, err == 0, opts);
		RETURN_IF_FALSE(MESH_SKELETON_FBX, fbx.skeleton_root_node != NULL
			, opts
			, "No skeleton in FBX source"
			);

		return parse_skeleton(as, fbx, fbx.skeleton_root_node, opts);
	}

} // namespace fbx

} // namespace crown

#endif // if CROWN_CAN_COMPILE
