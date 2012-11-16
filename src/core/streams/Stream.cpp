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

bool Stream::ZipTo(Stream* stream, size_t size, size_t& zippedSize)
{
	const size_t CHUNK_SIZE = 16384;
	int ret, flush;
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
		ReadDataBlock(in, this_step_bytes);

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
				stream->WriteDataBlock(out, have);
			
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */

		bytes_read += this_step_bytes;
		/* done when last data in file processed */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END);        /* stream will be complete */

	/* clean up and return */
	(void)deflateEnd(&strm);
	zippedSize = strm.total_out;
	return true;
}

bool Stream::UnzipTo(Stream* stream, size_t& /*unzippedSize*/)
{
	const size_t CHUNK_SIZE = 16384;
	int ret;
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

	size_t size = GetSize();
	size_t bytes_read = 0;

	/* decompress until deflate stream ends or end of file */
	do
	{
		size_t this_step_bytes = math::min(CHUNK_SIZE, size - bytes_read);
		ReadDataBlock(in, this_step_bytes);

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
					stream->WriteDataBlock(out, have);
		} while (strm.avail_out == 0);

		bytes_read += this_step_bytes;
		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END;
}

BinaryReader::BinaryReader(Stream* s)
{
	//BinaryReader takes the ownership of the stream.
	mStream = s;
}

BinaryReader::~BinaryReader()
{
}

uint8_t BinaryReader::ReadByte()
{
	uint8_t buffer;
	mStream->ReadDataBlock(&buffer, sizeof(uint8_t));
	return buffer;
}

short BinaryReader::ReadInt16()
{
	short buffer;
	mStream->ReadDataBlock(&buffer, sizeof(short));
	return buffer;
}

uint16_t BinaryReader::ReadUint16()
{
	uint16_t buffer;
	mStream->ReadDataBlock(&buffer, sizeof(uint16_t));
	return buffer;
}

int BinaryReader::ReadInt32()
{
	int buffer;
	mStream->ReadDataBlock(&buffer, sizeof(int));
	return buffer;
}

uint32_t BinaryReader::ReadUint32()
{
	uint32_t buffer;
	mStream->ReadDataBlock(&buffer, sizeof(uint32_t));
	return buffer;
}

long long BinaryReader::ReadInt64()
{
	long long buffer;
	mStream->ReadDataBlock(&buffer, sizeof(long long));
	return buffer;
}

double BinaryReader::ReadDouble()
{
	double buffer;
	mStream->ReadDataBlock(&buffer, sizeof(double));
	return buffer;
}

float BinaryReader::ReadFloat()
{
	float buffer;
	mStream->ReadDataBlock(&buffer, sizeof(float));
	return buffer;
}

BinaryWriter::BinaryWriter(Stream* s)
{
	//BinaryWriter takes the ownership of the stream.
	mStream = s;
}

BinaryWriter::~BinaryWriter()
{
}

void BinaryWriter::WriteByte(uint8_t buffer)
{
	mStream->WriteDataBlock(&buffer, sizeof(uint8_t));
}

void BinaryWriter::WriteInt16(short buffer)
{
	mStream->WriteDataBlock(&buffer, sizeof(short));
}

void BinaryWriter::WriteUint16(uint16_t buffer)
{
	mStream->WriteDataBlock(&buffer, sizeof(uint16_t));
}

void BinaryWriter::WriteInt32(int buffer)
{
	mStream->WriteDataBlock(&buffer, sizeof(int));
}

void BinaryWriter::WriteUint32(uint32_t buffer)
{
	mStream->WriteDataBlock(&buffer, sizeof(uint32_t));
}

void BinaryWriter::WriteInt64(long long buffer)
{
	mStream->WriteDataBlock(&buffer, sizeof(long long));
}

void BinaryWriter::WriteDouble(double buffer)
{
	mStream->WriteDataBlock(&buffer, sizeof(double));
}

void BinaryWriter::WriteFloat(float buffer)
{
	mStream->WriteDataBlock(&buffer, sizeof(float));
}

void BinaryWriter::InsertByte(uint8_t val, size_t offset)
{
	size_t tmpSize = mStream->GetSize() - offset;
	uint8_t* tmp = new uint8_t[tmpSize];
	mStream->Seek(offset, SM_SeekFromBegin);
	mStream->ReadDataBlock(tmp, tmpSize);
	mStream->Seek(offset, SM_SeekFromBegin);
	mStream->WriteByte(val);
	mStream->WriteDataBlock(tmp, tmpSize);
	delete[] tmp;
}

TextReader::TextReader(Stream* s)
{
	mStream = s;
}

TextReader::~TextReader()
{
}

char TextReader::ReadChar()
{
	return mStream->ReadByte();
}

char* TextReader::ReadString(char* string, int count)
{
	char currentChar;
	int i = 0;

	while(!mStream->EndOfStream() && i < count - 1)
	{
		currentChar = mStream->ReadByte();
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

TextWriter::TextWriter(Stream* s)
{
	mStream = s;
}

TextWriter::~TextWriter()
{
}

void TextWriter::WriteChar(char c)
{
	mStream->WriteByte(c);
}

void TextWriter::WriteString(const char* string)
{
	size_t count = 0;

	while(string[count] != '\0')
	{
		mStream->WriteByte(string[count]);
		count++;
	}
}

} // namespace crown

