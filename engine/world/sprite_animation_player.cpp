/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sprite_animation_player.h"
#include "array.h"
#include "memory.h"

namespace crown
{

SpriteAnimationPlayer::SpriteAnimationPlayer()
	: m_animations(default_allocator())
{
}

SpriteAnimation* SpriteAnimationPlayer::create_sprite_animation(const SpriteAnimationResource* sar)
{
	SpriteAnimation* anim = CE_NEW(default_allocator(), SpriteAnimation)(sar);
	array::push_back(m_animations, anim);
	return anim;
}

void SpriteAnimationPlayer::destroy_sprite_animation(SpriteAnimation* anim)
{
	const uint32_t num = array::size(m_animations);

	for (uint32_t i = 0; i < num; i++)
	{
		if (anim == m_animations[i])
		{
			CE_DELETE(default_allocator(), anim);
			m_animations[i] = m_animations[num - 1];
			array::pop_back(m_animations);
			return;
		}
	}
}

void SpriteAnimationPlayer::update(float dt)
{
	const uint32_t num = array::size(m_animations);

	for (uint32_t i = 0; i < num; i++)
	{
		m_animations[i]->update(dt);
	}
}

} // namespace crown
