/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/math/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"

namespace crown
{
struct AnimationKeyHeader
{
	enum Type
	{
		POSITION = 0, ///< Position data.
		ROTATION = 1, ///< Rotation data.

		COUNT
	};

	u32 type : 1;      ///< AnimationKeyHeader::Type
	u32 track_id : 10; ///< Track ID.
	u32 time : 16;     ///< Timestamp in milliseconds.
};
CE_STATIC_ASSERT(sizeof(AnimationKeyHeader) == 4);

struct PositionKey
{
	AnimationKeyHeader h;
	Vector3 value;
};

struct RotationKey
{
	AnimationKeyHeader h;
	Quaternion value;
};

union AnimationKey
{
	AnimationKeyHeader h;
	PositionKey p;
	RotationKey r;
};

struct MeshAnimationResource
{
	u32 version;
	u32 num_tracks;
	f32 total_time;
	u32 num_keys;
	u32 keys_offset;
	u32 _pad0;
	StringId64 target_skeleton;
	u32 num_bones;
	u32 bone_ids_offset;
	// AnimationKey animation_keys[num_keys]
	// u16 bone_ids[num_bones]
};

#if CROWN_CAN_COMPILE
namespace mesh_animation_resource_internal
{
	///
	s32 compile(CompileOptions &opts);

} // namespace mesh_animation_resource_internal
#endif

namespace mesh_animation_resource
{
	///
	const AnimationKey *animation_keys(const MeshAnimationResource *mar);

	///
	const u16 *bone_ids(const MeshAnimationResource *mar);

} // namespace mesh_animation_resource

} // namespace crown
