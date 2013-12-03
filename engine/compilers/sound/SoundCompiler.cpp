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

#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <vorbis/vorbisfile.h>

#include "Config.h"
#include "SoundCompiler.h"
#include "Allocator.h"
#include "Filesystem.h"
#include "StringUtils.h"
#include "DynamicString.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
SoundCompiler::SoundCompiler() 
	: m_sound_data_size(0)
	, m_sound_data(NULL)
{
}

//-----------------------------------------------------------------------------
SoundCompiler::~SoundCompiler()
{
}

//-----------------------------------------------------------------------------
size_t SoundCompiler::compile_impl(Filesystem& fs, const char* resource_path)
{
	size_t size = 0;
	
	size = compile_if_wav(fs, resource_path);

	if (size == 0)
	{
		size = compile_if_ogg(fs, resource_path);
	}
	
	return 1;
}

//-----------------------------------------------------------------------------
void SoundCompiler::write_impl(File* out_file)
{
	out_file->write((char*)&m_sound_header, sizeof(SoundHeader));
	out_file->write((char*)m_sound_data, m_sound_data_size);

	if (m_sound_data)
	{

		default_allocator().deallocate(m_sound_data);
		m_sound_data_size = 0;
		m_sound_data = NULL;
	}
}

//-----------------------------------------------------------------------------
size_t SoundCompiler::compile_if_wav(Filesystem& fs, const char* resource_path)
{
	File* in_file = fs.open(resource_path, FOM_READ);

	WAVHeader header;

	in_file->read((char*)&header, sizeof(WAVHeader));

	if (header.riff[0] != 'R' && header.riff[1] != 'I' && header.riff[2] != 'F' && header.riff[3] != 'F')
	{
		if (header.wave[0] != 'W' && header.wave[1] != 'A' && header.wave[2] != 'V' && header.wave[3] != 'E')
		{
			if (header.fmt[0] != 'f' && header.fmt[1] != 'm' && header.fmt[2] != 't' && header.fmt[3] != ' ')
			{
				fs.close(in_file);
				return 0;
			}
		}
	}

	m_sound_header.version = SOUND_VERSION;
	m_sound_header.size = header.data_size;
	m_sound_header.sample_rate = header.fmt_sample_rate;
	m_sound_header.avg_bytes_ps = header.fmt_avarage;
	m_sound_header.channels = header.fmt_channels;
	m_sound_header.block_size = header.fmt_block_align;
	m_sound_header.bits_ps = header.fmt_bits_ps;
	m_sound_header.sound_type = SoundType::WAV;

	m_sound_data_size = header.data_size;
	m_sound_data = (uint8_t*)default_allocator().allocate(m_sound_data_size);

	in_file->read((char*)m_sound_data, m_sound_data_size);

	fs.close(in_file);

	return sizeof(SoundHeader) + m_sound_data_size;
}

//-----------------------------------------------------------------------------
size_t SoundCompiler::compile_if_ogg(Filesystem& fs, const char* resource_path)
{
	// Retrieves resource absolute path
	DynamicString s(default_allocator());
	fs.get_absolute_path(resource_path, s);
	const char* abs_path = s.c_str();

	OggVorbis_File ogg_stream;

	bool result = ov_fopen(os::normalize_path(abs_path), &ogg_stream) == 0;

	if (result == false)
	{
		return 0;
	}

	vorbis_info* info = ov_info(&ogg_stream, -1);

	int64_t size = ov_raw_total(&ogg_stream, -1);
	int32_t rate = info->rate;
	int32_t channels = info->channels;

	ov_clear(&ogg_stream);

	File* in_file = fs.open(resource_path, FOM_READ);

	m_sound_header.version = SOUND_VERSION;
	m_sound_header.size = size;
	m_sound_header.sample_rate = rate;
	m_sound_header.block_size = (channels * 16) / 8;
	m_sound_header.avg_bytes_ps = rate * ((channels * 16) / 8);
	m_sound_header.channels = channels;
	m_sound_header.bits_ps = 16;
	m_sound_header.sound_type = SoundType::OGG;

	m_sound_data_size = size;
	m_sound_data = (uint8_t*)default_allocator().allocate(m_sound_data_size);

	in_file->read((char*)m_sound_data, m_sound_data_size);

	fs.close(in_file);

	return sizeof(SoundHeader) + m_sound_data_size;
}

} // namespace crown