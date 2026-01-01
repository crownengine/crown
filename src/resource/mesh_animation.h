/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE
#   include "core/memory/types.h"
#   include "core/strings/dynamic_string.h"
#   include "resource/mesh_animation_resource.h"
#   include "resource/types.h"

namespace crown
{
struct AnimationKeyIndex
{
	AnimationKeyHeader h;
	u32 offset; ///< Offset to first key.
	u32 num;    ///< Number of keys.
	u32 cur;    ///< Current key.
};

struct MeshAnimation
{
	Array<AnimationKey> sorted_keys;  ///< Animation keys sorted by access time.
	Array<AnimationKey> keys;         ///< Unordered animation keys.
	Array<AnimationKeyIndex> indices; ///< Indices into keys, sorted first by track_id then by type.
	u32 num_bones;                    ///< Number of bones affected by the animation.
	f32 total_time;                   ///< Animation duration in seconds.
	StringId64 target_skeleton;       ///< Reference to the animated skeleton.
	DynamicString stack_name;         ///< Animation name.
	HashMap<u16, u16> track_ids;      ///< From (bone_id, parameter_type) to track_id.
	Array<u16> bone_ids;              ///< From track_id to bone_id

	///
	explicit MeshAnimation(Allocator &a);
};

namespace mesh_animation
{
	/// Returns the track ID for the pair (bone_id, parameter_type).
	u16 track_id(MeshAnimation &a, u16 bone_id, u16 parameter_type);

	///
	s32 parse(MeshAnimation &ma, Buffer &buf, CompileOptions &opts);

} // namespace mesh_animation

} // namespace crown

#endif // if CROWN_CAN_COMPILE
