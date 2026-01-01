/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "world/event_stream.inl"
#include "world/sprite_animation_player.h"

namespace crown
{
namespace sprite_animation_player
{
	AnimationId create(SpriteAnimationPlayer &p, const SpriteAnimationResource *animation_resource)
	{
		SpriteAnimationPlayer::Index &index = p._indices[p._freelist_dequeue];
		p._freelist_dequeue = index.next;
		index.id += ANIMATION_ID_ADD;
		index.index = array::size(p._animations);

		SpriteAnimationPlayer::Animation a;
		a.id         = index.id;
		a.num_frames = animation_resource->num_frames;
		a.time_total = animation_resource->total_time;
		a.frames     = sprite_animation_resource::frames(animation_resource);
		a.resource   = animation_resource;
		array::push_back(p._animations, a);

		return a.id;
	}

	void destroy(SpriteAnimationPlayer &p, AnimationId anim_id)
	{
		SpriteAnimationPlayer::Index &index = p._indices[anim_id & ANIMATION_INDEX_MASK];

		SpriteAnimationPlayer::Animation &a = p._animations[index.index];
		a = p._animations[array::size(p._animations) - 1];
		array::pop_back(p._animations);
		p._indices[a.id & ANIMATION_INDEX_MASK].index = index.index;

		index.index = UINT32_MAX;
		p._indices[p._freelist_enqueue].next = anim_id & ANIMATION_INDEX_MASK;
		p._freelist_enqueue = anim_id & ANIMATION_INDEX_MASK;
	}

	bool has(SpriteAnimationPlayer &p, AnimationId anim_id)
	{
		SpriteAnimationPlayer::Index &index = p._indices[anim_id & ANIMATION_INDEX_MASK];
		return index.index != UINT32_MAX && index.id == anim_id;
	}

	void evaluate(SpriteAnimationPlayer &p, AnimationId anim_id, f32 time, UnitId unit, EventStream &events)
	{
		SpriteAnimationPlayer::Index &index = p._indices[anim_id & ANIMATION_INDEX_MASK];
		SpriteAnimationPlayer::Animation &a = p._animations[index.index];

		CE_ENSURE(time <= a.time_total);
		const f32 frame_ratio     = time / a.time_total;
		const u32 frame_unclamped = u32(frame_ratio * f32(a.num_frames));
		const u32 frame_index     = min(frame_unclamped, a.num_frames - 1);

		SpriteFrameChangeEvent ev;
		ev.unit      = unit;
		ev.frame_num = a.frames[frame_index];
		event_stream::write(events, 0, ev);
	}

} // namespace sprite_animation_player

SpriteAnimationPlayer::SpriteAnimationPlayer(Allocator &a)
	: _animations(a)
{
	for (u32 i = 0; i < countof(_indices); ++i) {
		_indices[i].id = i;
		_indices[i].next = i + 1;
		_indices[i].index = UINT32_MAX;
	}

	_freelist_dequeue = 0;
	_freelist_enqueue = countof(_indices) - 1;
}

} // namespace crown
