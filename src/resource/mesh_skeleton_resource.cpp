/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/constants.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/mesh_skeleton_fbx.h"
#include "resource/mesh_skeleton_resource.h"
#include "resource/resource_id.inl"

LOG_SYSTEM(MESH_SKELETON_RESOURCE, "mesh_skeleton_resource")

namespace crown
{
namespace mesh_skeleton_resource_internal
{
	s32 write(AnimationSkeleton &s, CompileOptions &opts)
	{
		MeshSkeletonResource asr;
		asr.version = RESOURCE_VERSION_MESH_SKELETON;
		asr.num_bones = array::size(s.local_transforms);
		asr.local_transforms_offset = sizeof(asr);
		asr.parents_offset = asr.local_transforms_offset + sizeof(BoneTransform) * asr.num_bones;
		asr.binding_matrices_offset = asr.parents_offset + sizeof(u32) * asr.num_bones;

		opts.write(asr.version);
		opts.write(asr.num_bones);
		opts.write(asr.local_transforms_offset);
		opts.write(asr.parents_offset);
		opts.write(asr.binding_matrices_offset);

		for (u32 i = 0; i < asr.num_bones; ++i)
			opts.write(s.local_transforms[i]);

		for (u32 i = 0; i < asr.num_bones; ++i)
			opts.write(s.parents[i]);

		for (u32 i = 0; i < asr.num_bones; ++i)
			opts.write(s.binding_matrices[i]);

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		AnimationSkeleton s(default_allocator());

		if (opts._resource_id._id == resource_id(RESOURCE_TYPE_MESH_SKELETON, STRING_ID_64("core/fallback/fallback", 0xd09058ae71962248))._id) {
			opts.read();

			BoneTransform bone_tm;
			bone_tm.position = VECTOR3_ZERO;
			bone_tm.rotation = QUATERNION_IDENTITY;
			bone_tm.scale = VECTOR3_ONE;

			for (u32 i = 0; i < MESH_SKELETON_MAX_BONES; ++i) {
				array::push_back(s.local_transforms, bone_tm);
				array::push_back(s.parents, (u32)UINT16_MAX);
				array::push_back(s.binding_matrices, MATRIX4X4_IDENTITY);
			}

			return write(s, opts);
		}

		s32 err = mesh_skeleton::parse(s, opts);
		ENSURE_OR_RETURN(MESH_SKELETON_RESOURCE, err == 0, opts);
		return write(s, opts);
	}

} // namespace mesh_skeleton_resource_internal

} // namespace crown
#endif // if CROWN_CAN_COMPILE
