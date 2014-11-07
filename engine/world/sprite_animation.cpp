/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sprite_animation.h"
#include "log.h"

namespace crown
{

SpriteAnimation::SpriteAnimation(const SpriteAnimationResource* sar)
	: m_resource(sar)
	, m_animation(NULL)
	, m_frames(sprite_animation_resource::get_animation_frames(sar))
	, m_cur_time(0)
	, m_cur_frame(0)
	, m_loop(false)
{
}

void SpriteAnimation::play(StringId32 name, bool loop)
{
	if (m_animation)
		return;

	m_animation = sprite_animation_resource::get_animation(m_resource, name);
	m_cur_time = 0;
	m_cur_frame = 0;
	m_loop = loop;
}

void SpriteAnimation::stop()
{
	m_animation = NULL;
	m_cur_time = 0;
	m_cur_frame = 0;
}

void SpriteAnimation::update(float dt)
{
	if (!m_animation)
		return;

	const uint32_t frame = m_animation->first_frame + uint32_t(m_animation->num_frames * (m_cur_time / m_animation->time));

	m_cur_frame = m_frames[frame];
	m_cur_time += dt;

	if (m_cur_time >= m_animation->time)
	{
		if (m_loop)
		{
			m_cur_time = 0;
			return;
		}
		else
		{
			stop();
			return;
		}
	}
}

} // namespace crown
