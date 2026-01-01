/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"
#include "resource/types.h"

namespace crown
{
struct SpriteResource
{
	u32 version;
	OBB obb;
	u32 num_frames;
	u32 num_verts;
	// verts[num_verts]
};

namespace sprite_resource
{
	/// Returns the frame data for the frame @a i.
	const f32 *frame_data(const SpriteResource *sr, u32 i);

} // namespace sprite_resource

struct SpriteAnimationResource
{
	u32 version;
	u32 num_frames;
	f32 total_time;
};

namespace sprite_animation_resource
{
	/// Returns the frames of the sprite animation @a sar.
	const u32 *frames(const SpriteAnimationResource *sar);

} // namespace sprite_animation_resource

} // namespace crown
