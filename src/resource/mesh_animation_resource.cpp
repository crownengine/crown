/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/mesh_animation_resource.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/globals.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "resource/compile_options.inl"
#include "resource/mesh_animation.h"
#include "resource/mesh_skeleton.h"

namespace crown
{
#if CROWN_CAN_COMPILE
namespace mesh_animation_resource_internal
{
	static s32 write(MeshAnimation &ma, CompileOptions &opts)
	{
		MeshAnimationResource mar;
		mar.version = RESOURCE_VERSION_MESH_ANIMATION;
		mar.num_tracks = hash_map::size(ma.track_ids);
		mar.total_time = ma.total_time;
		mar.num_keys = array::size(ma.sorted_keys);
		mar.keys_offset = sizeof(mar);
		mar._pad0 = 0u;
		mar.target_skeleton = ma.target_skeleton;
		mar.num_bones = array::size(ma.bone_ids);
		mar.bone_ids_offset = mar.keys_offset + mar.num_keys * sizeof(AnimationKey);

		opts.write(mar.version);
		opts.write(mar.num_tracks);
		opts.write(mar.total_time);
		opts.write(mar.num_keys);
		opts.write(mar.keys_offset);
		opts.write(mar._pad0);
		opts.write(mar.target_skeleton);
		opts.write(mar.num_bones);
		opts.write(mar.bone_ids_offset);

		for (u32 i = 0; i < array::size(ma.sorted_keys); ++i)
			opts.write(ma.sorted_keys[i]);

		for (u32 i = 0; i < array::size(ma.bone_ids); ++i)
			opts.write(ma.bone_ids[i]);

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		MeshAnimation ma(default_allocator());

		s32 err = mesh_animation::parse(ma, buf, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		return write(ma, opts);
	}

} // namespace mesh_animation_resource_internal
#endif // if CROWN_CAN_COMPILE

namespace mesh_animation_resource
{
	const AnimationKey *animation_keys(const MeshAnimationResource *mar)
	{
		return (AnimationKey *)((char *)mar + mar->keys_offset);
	}

	const u16 *bone_ids(const MeshAnimationResource *mar)
	{
		return (u16 *)((char *)mar + mar->bone_ids_offset);
	}

} // namespace mesh_animation_resource

} // namespace crown
