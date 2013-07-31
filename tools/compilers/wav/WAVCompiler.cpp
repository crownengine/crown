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

#include <cstdio>

#include "WAVCompiler.h"

namespace crown
{

//-----------------------------------------------------------------------------
WAVCompiler::WAVCompiler() :
	m_sound_data_size(0),
	m_sound_data(NULL)
{

}

//-----------------------------------------------------------------------------
WAVCompiler::~WAVCompiler()
{
	if (m_sound_data)
	{
		delete[] m_sound_data;
	}
}

//-----------------------------------------------------------------------------
size_t WAVCompiler::compile_impl(const char* resource_path)
{
	std::fstream in_file;
	in_file.open(resource_path, std::fstream::in | std::fstream::binary);

	if (!in_file.is_open())
	{
		printf("Unable to open file: %s\n", resource_path);
		return 0;
	}

	// Read the header
	if (!in_file.read((char*)(char*)&m_wav_header, sizeof(WAVHeader)))
	{
		printf("Unable to read the WAV header.");
		return 0;
	}

	m_sound_header.version = SOUND_VERSION;
	m_sound_header.size = m_wav_header.data_size;
	m_sound_header.channels = m_wav_header.fmt_channels;
	m_sound_header.bits_per_sample = m_wav_header.fmt_bits_per_sample;

	m_sound_data_size = m_wav_header.data_size;
	m_sound_data = new uint8_t[m_sound_data_size];

	in_file.read((char*)m_sound_data, m_sound_data_size);

	return sizeof(SoundHeader) + m_sound_data_size;
}

//-----------------------------------------------------------------------------
void WAVCompiler::write_impl(std::fstream& out_file)
{
	out_file.write((char*)&m_sound_header, sizeof(SoundHeader));
	out_file.write((char*)m_sound_data, m_sound_data_size);
}

} // namespace crown
