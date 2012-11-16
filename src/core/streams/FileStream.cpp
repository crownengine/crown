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

#include "FileStream.h"
#include "Types.h"
#include "Log.h"
#include "MathUtils.h"

namespace crown
{

FileStream::FileStream(StreamOpenMode openMode, const Str& filename) :
	Stream(openMode), mFile(0),
	mLastWasRead(true)
{
	//Takes ownership
	FileOpenMode fileOpenMode = (FileOpenMode)0;

	if (math::test_bitmask(openMode, SOM_READ))
		fileOpenMode = (FileOpenMode)(fileOpenMode | FOM_READ);
	if (math::test_bitmask(openMode, SOM_WRITE))
		fileOpenMode = (FileOpenMode)(fileOpenMode | FOM_WRITE);

	mFile = File::Open(filename.c_str(), fileOpenMode);
}

FileStream::~FileStream()
{
	delete mFile;
	mFile = 0;
}

uint8_t FileStream::ReadByte()
{
	CheckValid();

	if (!mLastWasRead)
	{
		mLastWasRead = true;
		fseek(mFile->GetHandle(), 0, SEEK_CUR);
	}

	uint8_t buffer;

	if (fread(&buffer, 1, 1, mFile->GetHandle()) != 1)
	{
		Log::E("Could not read from file.");
		throw FileIOException("Could not read from file.");
	}

	return buffer;
}

void FileStream::ReadDataBlock(void* buffer, size_t size)
{
	CheckValid();

	if (!mLastWasRead)
	{
		mLastWasRead = true;
		fseek(mFile->GetHandle(), 0, SEEK_CUR);
	}

	if (fread(buffer, size, 1, mFile->GetHandle()) != 1)
	{
		Log::E("Could not read from file.");
		throw FileIOException("Could not read from file.");
	}
}

bool FileStream::CopyTo(Stream* stream, size_t size)
{
	CheckValid();

	if (stream == 0)
		return false;
	if (size == 0)
		return true;

	const size_t chunksize = 1024*1024;

	char* buff = new char[chunksize];

	size_t totReadBytes = 0;

	while (totReadBytes < size)
	{
		int readBytes;
		int expectedReadBytes = math::min(size - totReadBytes, chunksize);
		readBytes = fread(buff, 1, expectedReadBytes, mFile->GetHandle());

		if (readBytes < expectedReadBytes)
		{
			if (feof(mFile->GetHandle()))
			{
				if (readBytes != 0)
				{
					stream->WriteDataBlock(buff, readBytes);
				}
			}

			delete[] buff;
			//Either the file gave an error, or ended before size bytes could be copied
			return false;
		}

		stream->WriteDataBlock(buff, readBytes);
		totReadBytes += readBytes;
	}

	delete [] buff;
	return true;
}

bool FileStream::EndOfStream() const
{
	return GetPosition() == GetSize();
}

void FileStream::WriteByte(uint8_t val)
{
	CheckValid();

	if (mLastWasRead)
	{
		mLastWasRead = false;
		fseek(mFile->GetHandle(), 0, SEEK_CUR);
	}

	if (fputc(val, mFile->GetHandle()) == EOF)
	{
		Log::E("Could not write to file.");
		throw FileIOException("Could not write to file.");
	}
}

void FileStream::WriteDataBlock(const void* buffer, size_t size)
{
	CheckValid();

	if (mLastWasRead)
	{
		mLastWasRead = false;
		fseek(mFile->GetHandle(), 0, SEEK_CUR);
	}

	if (fwrite(buffer, size, 1, mFile->GetHandle()) != 1)
	{
		Log::E("Could not write to file.");
		throw FileIOException("Could not write to file.");
	}
}

void FileStream::Flush()
{
	CheckValid();
	fflush(mFile->GetHandle());
}

void FileStream::Seek(int newPos, SeekMode mode)
{
	CheckValid();
	//flush(); <<<---?
	fseek(mFile->GetHandle(), newPos, (mode==SM_SeekFromBegin)?SEEK_SET:(mode==SM_SeekFromCurrent)?SEEK_CUR:SEEK_END);
}

size_t FileStream::GetPosition() const
{
	CheckValid();
	return ftell(mFile->GetHandle());
}

size_t FileStream::GetSize() const
{
	size_t pos = GetPosition();
	fseek(mFile->GetHandle(), 0, SEEK_END);
	size_t size = GetPosition();
	fseek(mFile->GetHandle(), pos, SEEK_SET);
	return size;
}

bool FileStream::CanRead() const
{
	CheckValid();
	return Stream::CanRead();
}

bool FileStream::CanWrite() const
{
	CheckValid();
	return Stream::CanWrite();
}

bool FileStream::CanSeek() const
{
	return true;
}

}
