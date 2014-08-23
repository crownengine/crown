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
