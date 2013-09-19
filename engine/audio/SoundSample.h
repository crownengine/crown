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
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
uint32_t ms_to_samples(uint32_t ms, uint32_t sample_rate)
{
	return (ms * (sample_rate / 100)) / 10; 
}

uint32_t samples_to_ms(uint32_t samples, uint32_t sample_rate)
{
	return sample_rate < 100 ? 0 : (samples * 10) / (sample_rate / 100);
}

//-----------------------------------------------------------------------------
enum WaveFormat
{
	MONO_8 = 0,
	STEREO_8,
	MONO_16,
	STEREO_16
};

//-----------------------------------------------------------------------------
struct WaveSample
{
	//-----------------------------------------------------------------------------
	void create(const void* data, const size_t size, const uint32_t sample_rate,
				 const uint32_t channels, const uint16_t block_size, const uint32_t bits_ps)
	{
		m_size = size;
		m_sample_rate = sample_rate;
		m_num_channels = channels;

		m_num_samples = m_size / block_size;

		Log::i("bits ps: %d", bits_ps);
		Log::i("channels: %d", channels);

		switch (bits_ps)
		{
			case 8:
			{
				m_format = channels == 2 ? STEREO_8 : MONO_8;
				break;
			}
			case 16:
			{
				m_format = channels == 2 ? STEREO_16 : MONO_16;
				break;
			}
			default:
			{
				CE_ASSERT(false, "Format not supported");
			}
		}

		m_data = (int16_t*)data;
	}

	//-----------------------------------------------------------------------------
	uint32_t length_in_ms()
	{
		return samples_to_ms(m_num_samples, m_sample_rate);
	}

public:

	int16_t*	m_data;
	WaveFormat	m_format;

	size_t		m_size;
	uint32_t	m_sample_rate;
	uint16_t	m_num_channels;
	uint32_t	m_num_samples;
};

} // namespace crown