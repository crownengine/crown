/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
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
