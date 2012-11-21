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
#include "zlib.h"
#include "MathUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
bool Stream::zip_to(Stream* stream, size_t size, size_t& zipped_size)
{
	const size_t CHUNK_SIZE = 16384;
	int32_t ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK_SIZE];
	unsigned char out[CHUNK_SIZE];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, 6);
	if (ret != Z_OK)
		return false;

	size_t bytes_read = 0;
	do
	{
		size_t this_step_bytes = math::min(CHUNK_SIZE, size - bytes_read);
		read_data_block(in, this_step_bytes);

		strm.avail_in = this_step_bytes;
		strm.next_in = in;

		flush = (size - bytes_read) <= CHUNK_SIZE ? Z_FINISH : Z_NO_FLUSH;

		do
		{
			strm.avail_out = CHUNK_SIZE;
			strm.next_out = out;

			ret = deflate(&strm, flush);    /* no bad return value */
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

			have = CHUNK_SIZE - strm.avail_out;
			if (have > 0)
				stream->write_data_block(out, have);
			
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */

		bytes_read += this_step_bytes;
		/* done when last data in file processed */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END);        /* stream will be complete */

	/* clean up and return */
	(void)deflateEnd(&strm);

	zipped_size = strm.total_out;

	return true;
}

//-----------------------------------------------------------------------------
bool Stream::unzip_to(Stream* stream, size_t& /*unzipped_size*/)
{
	const size_t CHUNK_SIZE = 16384;
	int32_t ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK_SIZE];
	unsigned char out[CHUNK_SIZE];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
			return false;

	size_t size = this->size();
	size_t bytes_read = 0;

	/* decompress until deflate stream ends or end of file */
	do
	{
		size_t this_step_bytes = math::min(CHUNK_SIZE, size - bytes_read);
		read_data_block(in, this_step_bytes);

		strm.avail_in = this_step_bytes;
		strm.next_in = in;
		if (strm.avail_in == 0)
				break;

		/* run inflate() on input until output buffer not full */
		do {
				strm.avail_out = CHUNK_SIZE;
				strm.next_out = out;
				ret = inflate(&strm, Z_NO_FLUSH);
				assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
				switch (ret) {
				case Z_NEED_DICT:
						ret = Z_DATA_ERROR;     /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
						(void)inflateEnd(&strm);
						return false;
				}
				have = CHUNK_SIZE - strm.avail_out;
				if (have > 0)
					stream->write_data_block(out, have);
		} while (strm.avail_out == 0);

		bytes_read += this_step_bytes;
		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END;
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
	m_stream->read_data_block(&buffer, sizeof(int8_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int16_t BinaryReader::read_int16()
{
	int16_t buffer;
	m_stream->read_data_block(&buffer, sizeof(int16_t));
	return buffer;
}

//-----------------------------------------------------------------------------
uint16_t BinaryReader::read_uint16()
{
	uint16_t buffer;
	m_stream->read_data_block(&buffer, sizeof(uint16_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int32_t BinaryReader::read_int32()
{
	int32_t buffer;
	m_stream->read_data_block(&buffer, sizeof(int32_t));
	return buffer;
}

//-----------------------------------------------------------------------------
uint32_t BinaryReader::read_uint32()
{
	uint32_t buffer;
	m_stream->read_data_block(&buffer, sizeof(uint32_t));
	return buffer;
}

//-----------------------------------------------------------------------------
int64_t BinaryReader::read_int64()
{
	int64_t buffer;
	m_stream->read_data_block(&buffer, sizeof(int64_t));
	return buffer;
}

//-----------------------------------------------------------------------------
double BinaryReader::read_double()
{
	double buffer;
	m_stream->read_data_block(&buffer, sizeof(double));
	return buffer;
}

float BinaryReader::read_float()
{
	float buffer;
	m_stream->read_data_block(&buffer, sizeof(float));
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
	m_stream->write_data_block(&buffer, sizeof(int8_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int16(int16_t buffer)
{
	m_stream->write_data_block(&buffer, sizeof(int16_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_uint16(uint16_t buffer)
{
	m_stream->write_data_block(&buffer, sizeof(uint16_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int32(int32_t buffer)
{
	m_stream->write_data_block(&buffer, sizeof(int32_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_uint32(uint32_t buffer)
{
	m_stream->write_data_block(&buffer, sizeof(uint32_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_int64(int64_t buffer)
{
	m_stream->write_data_block(&buffer, sizeof(int64_t));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_double(double buffer)
{
	m_stream->write_data_block(&buffer, sizeof(double));
}

//-----------------------------------------------------------------------------
void BinaryWriter::write_float(float buffer)
{
	m_stream->write_data_block(&buffer, sizeof(float));
}

//-----------------------------------------------------------------------------
void BinaryWriter::insert_byte(int8_t val, size_t offset)
{
	size_t tmpSize = m_stream->size() - offset;
	int8_t* tmp = new int8_t[tmpSize];
	m_stream->seek(offset, SM_FROM_BEGIN);
	m_stream->read_data_block(tmp, tmpSize);
	m_stream->seek(offset, SM_FROM_BEGIN);
	m_stream->write_byte(val);
	m_stream->write_data_block(tmp, tmpSize);
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

