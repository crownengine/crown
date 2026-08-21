/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_skeleton_gltf.h"

#if CROWN_CAN_COMPILE
#   include "core/containers/array.inl"
#   include "core/containers/vector.inl"
#   include "core/math/matrix4x4.inl"
#   include "core/memory/globals.h"
#   include "device/log.h"
#   include "resource/compile_options.inl"
#   include "resource/gltf_document.h"
#   include <cgltf.h>

LOG_SYSTEM(MESH_SKELETON_GLTF, "mesh_skeleton_gltf")

namespace crown
{
namespace gltf
{
	s32 parse(AnimationSkeleton &s, const char *path, const char *skin_name, CompileOptions &opts)
	{
		GLTFDocument doc(default_allocator());
		s32 err = gltf::parse(doc, path, opts);
		ENSURE_OR_RETURN(MESH_SKELETON_GLTF, err == 0, opts);
		err = select_skin(doc, skin_name, opts);
		ENSURE_OR_RETURN(MESH_SKELETON_GLTF, err == 0, opts);
		RETURN_IF_FALSE(MESH_SKELETON_GLTF, vector::size(doc.bones) != 0, opts
			, "No skeleton in glTF source"
			);
		const Matrix4x4 root_transform = skeleton_root_transform(doc);
		const bool synthetic_root = doc.bones[0] == NULL;

		for (u32 i = 0; i < vector::size(doc.bones); ++i) {
			cgltf_node *node = doc.bones[i];
			if (node == NULL) {
				BoneTransform bone;
				bone.position = VECTOR3_ZERO;
				bone.rotation = QUATERNION_IDENTITY;
				bone.scale = VECTOR3_ONE;
				array::push_back(s.local_transforms, bone);
				array::push_back(s.parents, (u32)UINT16_MAX);
				array::push_back(s.binding_matrices, MATRIX4X4_IDENTITY);
				continue;
			}

			Matrix4x4 local = local_transform(node);
			u16 parent = node->parent != NULL ? bone_id(doc, node->parent) : UINT16_MAX;
			if (parent == UINT16_MAX) {
				local *= root_transform;
				if (synthetic_root)
					parent = 0;
			}

			BoneTransform bone;
			bone.position = translation(local);
			bone.rotation = rotation(local);
			bone.scale = scale(local);
			array::push_back(s.local_transforms, bone);

			array::push_back(s.parents, (u32)parent);

			Matrix4x4 binding = MATRIX4X4_IDENTITY;
			const s32 ji = doc.skin != NULL ? gltf::joint_index(*doc.skin, node) : -1;
			if (ji >= 0) {
				err = gltf::binding_matrix(binding, *doc.skin, (u32)ji, opts);
				ENSURE_OR_RETURN(MESH_SKELETON_GLTF, err == 0, opts);
			}
			array::push_back(s.binding_matrices, binding);
		}

		return 0;
	}

} // namespace gltf

} // namespace crown

#endif // if CROWN_CAN_COMPILE
