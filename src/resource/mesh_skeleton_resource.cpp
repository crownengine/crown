/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "resource/mesh_skeleton_resource.h"
#include "core/containers/array.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "resource/mesh_skeleton_fbx.h"
#include "resource/compile_options.inl"

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
		s32 err = mesh_skeleton::parse(s, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		return write(s, opts);
	}

} // namespace mesh_skeleton_resource_internal

} // namespace crown
#endif // if CROWN_CAN_COMPILE
