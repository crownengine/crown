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
#include "SoundCompiler.h"

namespace crown
{

//-----------------------------------------------------------------------------
SoundCompiler::SoundCompiler() :
	m_sound_data_size(0),
	m_sound_data(NULL)
{

}

//-----------------------------------------------------------------------------
SoundCompiler::~SoundCompiler()
{
	if (m_sound_data)
	{
		delete[] m_sound_data;
	}
}

//-----------------------------------------------------------------------------
size_t SoundCompiler::compile_impl(const char* resource_path)
{
	if (is_wav(resource_path))
	{
		compile_wav(resource_path);
	}
	else if(is_ogg(resource_path))
	{
		compile_ogg(resource_path);
	}
	else
	{
		printf("Wrong File. Abort.\n");
	}
}

//-----------------------------------------------------------------------------
void SoundCompiler::write_impl(std::fstream& out_file)
{
	out_file.write((char*)&m_sound_header, sizeof(SoundHeader));
	out_file.write((char*)m_sound_data, m_sound_data_size);
}

//-----------------------------------------------------------------------------
bool SoundCompiler::is_wav(const char* resource_path) const
{
	bool result = false;

	WAVHeader header;

	std::fstream in_file;
	in_file.open(resource_path, std::fstream::in | std::fstream::binary);

	if (!in_file.is_open())
	{
		printf("Unable to open file: %s\n", resource_path);
		return 0;
	}
	// Read the header
	if (!in_file.read((char*)&header, sizeof(WAVHeader)))
	{
		printf("Unable to read file: %s\n", resource_path);
		return 0;
	}

	if (header.riff[0] == 'R' && header.riff[1] == 'I' && header.riff[2] == 'F' && header.riff[3] == 'F')
	{
		if (header.wave[0] == 'W' && header.wave[1] == 'A' && header.wave[2] == 'V' && header.wave[3] == 'E')
		{
			if (header.fmt[0] == 'f' && header.fmt[1] == 'm' && header.fmt[2] == 't' && header.fmt[3] == ' ')
			{
				result = true;
			}
		}
	}

	in_file.close();

	return result;
}

//-----------------------------------------------------------------------------
bool SoundCompiler::is_ogg(const char* resource_path) const
{
	bool result = false;

	FILE* in_file;

	if ((in_file = fopen(resource_path, "rb")) == NULL)
	{
		printf("Unable to open file: %s\n", resource_path);
		return false;	
	}

	OggVorbis_File ogg_stream;

	result = ov_open(in_file, &ogg_stream, NULL, 0)	== 0;

	fclose(in_file);

	return result;
}

//-----------------------------------------------------------------------------
size_t SoundCompiler::compile_wav(const char* resource_path)
{
	FILE* in_file;

	if ((in_file = fopen(resource_path, "rb")) == NULL)
	{
		printf("Unable to open file: %s\n", resource_path);
		return 0;	
	}

	WAVHeader header;

	fread((char*)&header, sizeof(WAVHeader), 1, in_file);

	m_sound_header.version = SOUND_VERSION;
	m_sound_header.size = header.data_size;
	m_sound_header.sample_rate = header.fmt_sample_rate;
	m_sound_header.channels = header.fmt_channels;
	m_sound_header.bits_per_sample = header.fmt_bits_per_sample;
	m_sound_header.sound_type = ST_WAV;

	m_sound_data_size = header.data_size;
	m_sound_data = new uint8_t[m_sound_data_size];

	fread((char*)m_sound_data, m_sound_data_size, 1, in_file);

	return sizeof(SoundHeader) + m_sound_data_size;
}

//-----------------------------------------------------------------------------
size_t SoundCompiler::compile_ogg(const char* resource_path)
{
	FILE* in_file;

	if ((in_file = fopen(resource_path, "rb")) == NULL)
	{
		printf("Unable to open file: %s\n", resource_path);
		return 0;	
	}

	OggVorbis_File ogg_stream;

	ov_open(in_file, &ogg_stream, NULL, 0)	== 0;

	vorbis_info* info = ov_info(&ogg_stream, -1);

	int64_t size = ov_raw_total(&ogg_stream, -1);
	int32_t rate = info->rate;
	int32_t channels = info->channels;

	ov_clear(&ogg_stream);

	if ((in_file = fopen(resource_path, "rb")) == NULL)
	{
		printf("Unable to open file: %s\n", resource_path);
		return 0;	
	}

	m_sound_header.version = SOUND_VERSION;
	m_sound_header.size = size;
	m_sound_header.sample_rate = rate;
	m_sound_header.channels = channels;
	m_sound_header.bits_per_sample = 16;
	m_sound_header.sound_type = ST_OGG;

	m_sound_data_size = size;
	m_sound_data = new uint8_t[m_sound_data_size];

	fread((char*)m_sound_data, m_sound_data_size, 1, in_file);

	return sizeof(SoundHeader) + m_sound_data_size;
}

} // namespace crown
