/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/constants.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
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
	static void write_header(CompileOptions &opts, const MeshSkeletonResource &asr)
	{
		opts.write(asr.version);
		opts.write(asr.num_bones);
		opts.write(asr.local_transforms_offset);
		opts.write(asr.parents_offset);
		opts.write(asr.binding_matrices_offset);
		opts.write(asr._pad);
		opts.write(asr.mesh_resource);
	}

	s32 write(AnimationSkeleton &s, CompileOptions &opts)
	{
		MeshSkeletonResource asr = {};
		asr.version = RESOURCE_VERSION_MESH_SKELETON;
		asr.num_bones = array::size(s.local_transforms);
		asr._pad = 0u;
		asr.mesh_resource = StringId64(s.mesh_resource_name.c_str());

		write_header(opts, asr);

		opts.align(alignof(BoneTransform));
		asr.local_transforms_offset = opts._output.position();
		for (u32 i = 0; i < asr.num_bones; ++i) {
			opts.write(s.local_transforms[i].position);
			opts.write(s.local_transforms[i].rotation);
			opts.write(s.local_transforms[i].scale);
		}

		opts.align(alignof(u32));
		asr.parents_offset = opts._output.position();
		opts.write(s.parents);

		opts.align(alignof(Matrix4x4));
		asr.binding_matrices_offset = opts._output.position();
		for (u32 i = 0; i < asr.num_bones; ++i)
			opts.write(s.binding_matrices[i]);

		const u32 end = opts._output.position();
		opts._output.seek(0);
		write_header(opts, asr);
		opts._output.seek(end);

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		AnimationSkeleton s(default_allocator());

		if (opts._resource_id._id == resource_id(RESOURCE_TYPE_MESH_SKELETON, STRING_ID_64("core/fallback/fallback", 0xd09058ae71962248))._id) {
			opts.read();
			s.mesh_resource_name = "core/fallback/fallback";
			opts.add_requirement("mesh", "core/fallback/fallback");

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
