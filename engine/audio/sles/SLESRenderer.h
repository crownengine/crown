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

#include "Assert.h"
#include "Types.h"

namespace crown
{


//-----------------------------------------------------------------------------
struct SoundBuffer
{

	void create(const void* data, const uint32_t size, const uint32_t sample_rate, const uint32_t channels, const uint32_t bits)
	{
		CE_ASSERT_NOT_NULL(data);
		CE_ASSERT(size > 0, "Size must be > 0");

		m_data = (uint16_t*)data;
		m_size = size;
		m_sample_rate = sample_rate;
		m_channels = channels;
		m_bits_per_sample = bits;
	}

	void update(const void* data, const uint32_t size)
	{
		CE_ASSERT_NOT_NULL(data);
		CE_ASSERT(size > 0, "Size must be > 0");

		m_data = (uint16_t*)data;
		m_size = size;
	}

	void destroy()
	{

	}

public:

	uint16_t* 						m_data;
	size_t	  						m_size;
	uint32_t  						m_sample_rate;
	uint32_t  						m_channels;
	uint32_t  						m_bits_per_sample;
};

//-----------------------------------------------------------------------------
struct SoundSource
{
  	SLObjectItf 					m_player_obj;
  	SLPlayItf 						m_player_play;
  	SLAndroidSimpleBufferQueueItf 	m_player_bufferqueue;
  	SLSeekItf 						m_player_seek;
  	SLVolumeItf						m_player_volume;

  	bool							m_loop;
};

} // namespace crown