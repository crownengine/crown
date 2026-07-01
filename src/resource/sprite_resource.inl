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

	const u16 *event_times(const SpriteAnimationResource *sar)
	{
		return (u16 *)((char *)sar + sar->event_times_offset);
	}

	const StringId32 *event_names(const SpriteAnimationResource *sar)
	{
		return (StringId32 *)((char *)sar + sar->event_names_offset);
	}

} // namespace sprite_animation_resource

} // namespace crown
