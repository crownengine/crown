/*
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

#include "Stream.h"
#include "Types.h"
#include "Compressor.h"
#include "MallocAllocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
bool Stream::compress_to(Stream* stream, size_t size, size_t& zipped_size, Compressor* compressor)
{
	assert(stream != NULL);
	assert(compressor != NULL);

	MallocAllocator allocator;
	void* in_buffer = (void*)allocator.allocate(size);

	read(in_buffer, size);

	void* compressed_buffer = compressor->compress(in_buffer, size, zipped_size);

	stream->write(compressed_buffer, zipped_size);

	return true;
}

//-----------------------------------------------------------------------------
bool Stream::uncompress_to(Stream* stream, size_t& unzipped_size, Compressor* compressor)
{
	assert(stream != NULL);
	assert(compressor != NULL);

	MallocAllocator allocator;

	size_t stream_size = size();
	void* in_buffer = (void*)allocator.allocate(stream_size); 

	read(in_buffer, stream_size);

	void* uncompressed_buffer = compressor->uncompress(in_buffer, stream_size, unzipped_size);

	stream->write(uncompressed_buffer, unzipped_size);

	return true;
}

//-----------------------------------------------------------------------------
BinaryReader::BinaryReader(Stream* s)
{
	//BinaryReader takes the ownership of the stream.
	m_stream = s;
}

//-----------------------------------------------------------------------------
BinaryReader::~BinaryReader()
{
}

//-----------------------------------------------------------------------------
int8_t BinaryReader::read_byte()
{
	int8_t buffer;
	m_stream->read(&buffer, sizeof(int8_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int16_t BinaryReader::read_int16()
{
	int16_t buffer;
	m_stream->read(&buffer, sizeof(int16_t));
	return buffer;
}

//-----------------------------------------------------------------------------
uint16_t BinaryReader::read_uint16()
{
	uint16_t buffer;
	m_stream->read(&buffer, sizeof(uint16_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int32_t BinaryReader::read_int32()
{
	int32_t buffer;
	m_stream->read(&buffer, sizeof(int32_t));
	return buffer;
}

//-----------------------------------------------------------------------------
uint32_t BinaryReader::read_uint32()
{
	uint32_t buffer;
	m_stream->read(&buffer, sizeof(uint32_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int64_t BinaryReader::read_int64()
{
	int64_t buffer;
	m_stream->read(&buffer, sizeof(int64_t));
	return buffer;
}

//-----------------------------------------------------------------------------
double BinaryReader::read_double()
{
	double buffer;
	m_stream->read(&buffer, sizeof(double));
	return buffer;
}

float BinaryReader::read_float()
{
	float buffer;
	m_stream->read(&buffer, sizeof(float));
	return buffer;
}

//-----------------------------------------------------------------------------
BinaryWriter::BinaryWriter(Stream* s)
{
	//BinaryWriter takes the ownership of the stream.
	m_stream = s;
}

//-----------------------------------------------------------------------------
BinaryWriter::~BinaryWriter()
{
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_byte(int8_t buffer)
{
	m_stream->write(&buffer, sizeof(int8_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int16(int16_t buffer)
{
	m_stream->write(&buffer, sizeof(int16_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_uint16(uint16_t buffer)
{
	m_stream->write(&buffer, sizeof(uint16_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int32(int32_t buffer)
{
	m_stream->write(&buffer, sizeof(int32_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_uint32(uint32_t buffer)
{
	m_stream->write(&buffer, sizeof(uint32_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int64(int64_t buffer)
{
	m_stream->write(&buffer, sizeof(int64_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_double(double buffer)
{
	m_stream->write(&buffer, sizeof(double));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_float(float buffer)
{
	m_stream->write(&buffer, sizeof(float));
}

//-----------------------------------------------------------------------------
void BinaryWriter::insert_byte(int8_t val, size_t offset)
{
	size_t tmpSize = m_stream->size() - offset;
	int8_t* tmp = new int8_t[tmpSize];

	m_stream->seek(offset);
	m_stream->read(tmp, tmpSize);

	m_stream->seek(offset);

	m_stream->write_byte(val);

	m_stream->write(tmp, tmpSize);

	delete[] tmp;
}

//-----------------------------------------------------------------------------
TextReader::TextReader(Stream* s)
{
	m_stream = s;
}

//-----------------------------------------------------------------------------
TextReader::~TextReader()
{
}

//-----------------------------------------------------------------------------
char TextReader::read_char()
{
	return m_stream->read_byte();
}

//-----------------------------------------------------------------------------
char* TextReader::read_string(char* string, uint32_t count)
{
	char currentChar;
	int32_t i = 0;

	while(!m_stream->end_of_stream() && i < count - 1)
	{
		currentChar = m_stream->read_byte();
		string[i] = currentChar;

		i++;

		if (currentChar == '\n')
		{
			break;
		}
	}

	if (i == 0)
	{
		return NULL;
	}

	string[i] = '\0';

	return string;
}

//-----------------------------------------------------------------------------
TextWriter::TextWriter(Stream* s)
{
	m_stream = s;
}

//-----------------------------------------------------------------------------
TextWriter::~TextWriter()
{
}

void TextWriter::write_char(char c)
{
	m_stream->write_byte(c);
}

//-----------------------------------------------------------------------------
void TextWriter::write_string(const char* string)
{
	size_t count = 0;

	while(string[count] != '\0')
	{
		m_stream->write_byte(string[count]);
		count++;
	}
}

} // namespace crown

