/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "resource/mesh_animation_resource.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/globals.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/mesh_animation.h"
#include "resource/mesh_skeleton.h"
#include "resource/resource_id.inl"

LOG_SYSTEM(MESH_ANIMATION_RESOURCE, "mesh_animation_resource")

namespace crown
{
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
		mar.num_events = array::size(ma.events);
		mar.event_times_offset = mar.bone_ids_offset + mar.num_bones * sizeof(u16);
		mar.event_names_offset = (u32)(uintptr_t)memory::align_top((void *)(uintptr_t)(mar.event_times_offset + mar.num_events * sizeof(u16)), sizeof(u32));
		mar._pad1 = 0u;

		opts.write(mar.version);
		opts.write(mar.num_tracks);
		opts.write(mar.total_time);
		opts.write(mar.num_keys);
		opts.write(mar.keys_offset);
		opts.write(mar._pad0);
		opts.write(mar.target_skeleton);
		opts.write(mar.num_bones);
		opts.write(mar.bone_ids_offset);
		opts.write(mar.num_events);
		opts.write(mar.event_times_offset);
		opts.write(mar.event_names_offset);
		opts.write(mar._pad1);

		for (u32 i = 0; i < array::size(ma.sorted_keys); ++i)
			opts.write(ma.sorted_keys[i]);

		for (u32 i = 0; i < array::size(ma.bone_ids); ++i)
			opts.write(ma.bone_ids[i]);

		for (u32 i = 0; i < array::size(ma.events); ++i)
			opts.write(u16(ma.events[i].time * 1000.0f));

		opts.align(sizeof(u32));
		for (u32 i = 0; i < array::size(ma.events); ++i)
			opts.write(ma.events[i].name);

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		MeshAnimation ma(default_allocator());

		if (opts._resource_id._id == resource_id(RESOURCE_TYPE_MESH_ANIMATION, STRING_ID_64("core/fallback/fallback", 0xd09058ae71962248))._id) {
			ma.total_time = 1.0f;
			return write(ma, opts);
		}

		s32 err = mesh_animation::parse(ma, buf, opts);
		ENSURE_OR_RETURN(MESH_ANIMATION_RESOURCE, err == 0, opts);
		return write(ma, opts);
	}

} // namespace mesh_animation_resource_internal

} // namespace crown
#endif // if CROWN_CAN_COMPILE
