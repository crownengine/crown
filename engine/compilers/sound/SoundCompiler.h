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

#include <cstdio>

#include "Compiler.h"
#include "SoundResource.h"


namespace crown
{

//-----------------------------------------------------------------------------
struct WAVHeader
{
	char 			riff[4];				// Should contains 'RIFF'
	int32_t			chunk_size;				// Not Needed
	char 			wave[4];				// Should contains 'WAVE'
	char 			fmt[4];					// Should contains 'fmt '
	int32_t			fmt_size;				// Size of format chunk
	int16_t			fmt_tag;				// Identifies way data is stored, 1 means no compression
	int16_t			fmt_channels;			// Channel, 1 means mono, 2 means stereo
	int32_t			fmt_sample_rate;		// Sample per second
	int32_t			fmt_avarage;			// Avarage bytes per sample
	int16_t			fmt_block_align;		// Block alignment
	int16_t			fmt_bits_ps;			// Number of bits per sample
	char 			data[4];				// Should contains 'data'
	int32_t			data_size;				// Data dimension
};

//-----------------------------------------------------------------------------
class SoundCompiler : public Compiler
{
public:

						SoundCompiler();
						~SoundCompiler();

	size_t				compile_impl(Filesystem& fs, const char* resource_path);
	void				write_impl(File* out_file);

private:

	size_t				compile_if_wav(Filesystem& fs, const char* resource_path);
	size_t				compile_if_ogg(Filesystem& fs, const char* resource_path);

private:

	SoundHeader			m_sound_header;
	size_t				m_sound_data_size;
	uint8_t*			m_sound_data;
};

} // namespace crown