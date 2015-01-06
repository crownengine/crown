/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "sprite_resource.h"
#include "container_types.h"
#include "sprite_animation.h"

namespace crown
{

struct SpriteAnimationPlayer
{
	SpriteAnimationPlayer();

	SpriteAnimation* create_sprite_animation(const SpriteAnimationResource* sar);
	void destroy_sprite_animation(SpriteAnimation* anim);

	void update(float dt);

private:

	Array<SpriteAnimation*> m_animations;
};

} // namespace crown
