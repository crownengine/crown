/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "sprite_resource.h"

namespace crown
{

struct SpriteAnimation
{
	SpriteAnimation(const SpriteAnimationResource* sar);

	void play(StringId32 name, bool loop);
	void stop();

	void update(float dt);

public:

	const SpriteAnimationResource* m_resource;
	const SpriteAnimationData* m_animation;
	const uint32_t* m_frames;
	float m_cur_time;
	uint32_t m_cur_frame;
	bool m_loop;
};

} // namespace crown
