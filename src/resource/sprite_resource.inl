/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/sprite_resource.h"

namespace crown
{
namespace sprite_resource
{
	const f32 *frame_data(const SpriteResource *sr, u32 i)
	{
		CE_ENSURE(i < sr->num_frames);
		return ((f32 *)&sr[1]) + 20*i;
	}

} // namespace sprite_resource

namespace sprite_animation_resource
{
	const u32 *frames(const SpriteAnimationResource *sar)
	{
		return (u32 *)&sar[1];
	}

} // namespace sprite_animation_resource

} // namespace crown
