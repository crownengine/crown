/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "resource/sprite_resource.h"
#include "world/event_stream.h"
#include "world/types.h"

namespace crown
{
struct SpriteFrameChangeEvent
{
	UnitId unit;
	u32 frame_num;
};

#define MAX_ANIMATIONS       1024
#define ANIMATION_INDEX_MASK (MAX_ANIMATIONS - 1)
#define ANIMATION_ID_ADD     MAX_ANIMATIONS

struct SpriteAnimationPlayer
{
	struct Index
	{
		AnimationId id;
		u32 index;      ///< Index into _animations.
		u32 next;       ///< Next free index slot.
	};

	struct Animation
	{
		AnimationId id;
		u32 num_frames;
		f32 time_total;
		const u32 *frames;
		const SpriteAnimationResource *resource;
	};

	u32 _freelist_dequeue;
	u32 _freelist_enqueue;
	Index _indices[MAX_ANIMATIONS];
	Array<Animation> _animations;

	///
	explicit SpriteAnimationPlayer(Allocator &a);
};

namespace sprite_animation_player
{
	///
	AnimationId create(SpriteAnimationPlayer &p, const SpriteAnimationResource *animation_resource);

	///
	void destroy(SpriteAnimationPlayer &p, AnimationId anim_id);

	///
	bool has(SpriteAnimationPlayer &p, AnimationId anim_id);

	///
	void evaluate(SpriteAnimationPlayer &p, AnimationId anim_id, f32 time, UnitId unit, EventStream &events);

} // namespace sprite_animation_player

} // namespace crown
