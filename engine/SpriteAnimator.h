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

#pragma once

#include <cstring>

#include "Assert.h"
#include "SpriteResource.h"
#include "Device.h"
#include "RendererTypes.h"
#include "Renderer.h"
#include "Random.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
class SpriteAnimator
{
public:
	//-----------------------------------------------------------------------------
	enum Enum
	{
		UNKNOWN = 0,
		SEQUENTIAL_ONCE,
		SEQUENTIAL_LOOP,
		RANDOM_LOOP
	};

public:
	//-----------------------------------------------------------------------------	
	SpriteAnimator(const SpriteResource* sr)
		: m_vb(sr->vertex_buffer())
		, m_num_frames(sr->num_frames())
		, m_frame_rate(sr->frame_rate())
		, m_playback_mode((Enum)sr->playback_mode())
		, m_cur_frame(0)
		, m_random(os::microseconds())
	{
		memcpy(m_vertices, sr->animation(), 16 * 4 * m_num_frames);
	}

	//-----------------------------------------------------------------------------
	void play_frame()
	{
		CE_ASSERT(m_playback_mode != UNKNOWN, "Playback mode must be != UNKNOWN (%d)", m_playback_mode);

		update_frame();

		device()->renderer()->update_vertex_buffer(m_vb, 0, 4, m_vertices + 16 * m_cur_frame);
	}

	//-----------------------------------------------------------------------------
	void update_frame()
	{
		switch (m_playback_mode)
		{
			case SEQUENTIAL_ONCE:
			{
				m_cur_frame = ++m_cur_frame < m_num_frames ? m_cur_frame : m_num_frames - 1;
				break;
			}
			case SEQUENTIAL_LOOP:
			{
				m_cur_frame = ++m_cur_frame < m_num_frames ? m_cur_frame : 0;
				break;
			}
			case RANDOM_LOOP:
			{
				m_cur_frame = m_random.integer(m_num_frames);
				break;
			}
			default:
			{
				CE_ASSERT(false, "Ops! Wrong playback mode!");
			}
		}
	}

public:

	VertexBufferId	m_vb;
	float			m_vertices[MAX_SPRITE_ANIM_FRAMES*SPRITE_FRAME_SIZE];

	uint32_t		m_num_frames;
	uint32_t		m_frame_rate;
	Enum			m_playback_mode;
	uint32_t		m_cur_frame;

	Random 			m_random;
};

} // namespace crown 