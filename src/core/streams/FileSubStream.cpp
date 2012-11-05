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

#include "FileSubStream.h"
#include "Types.h"
#include "Log.h"

namespace crown
{

FileSubStream::FileSubStream(StreamOpenMode openMode, const Str& filename, size_t position, size_t length) :
	Stream(openMode), mFile(0),
	mSubPosition(position), mSubLength(length)
{
	//Takes ownership
	
	if (openMode != SOM_READ)
		Log::E("FileSubStream accepts only SOM_READ as openMode.");

	mFile = File::Open(filename.c_str(), FOM_READ);
	size_t fileSize = mFile->GetSize();
	if (position >= fileSize || position + length >= fileSize)
		throw ArgumentException("Position and length parameters define a file subsection outside the file dimensions");
	fseek(mFile->GetHandle(), position, SEEK_SET);
}

FileSubStream::~FileSubStream()
{
	delete mFile;
	mFile = 0;
}

uchar FileSubStream::ReadByte()
{
	CheckValid();

	uchar buffer;
	
	bool error = false;
	
	if (EndOfStream())
		error = true;
	else
		error = fread(&buffer, 1, 1, mFile->GetHandle()) != 1;

	if (error)
	{
		Log::E("Could not read from file.");
		throw FileIOException("Could not read from file.");
	}

	return buffer;
}

void FileSubStream::ReadDataBlock(void* buffer, size_t size)
{
	CheckValid();

	bool error = false;
	
	if (GetPosition() + size > GetSize())
		error = true;
	else
		error = fread(buffer, size, 1, mFile->GetHandle()) != 1;

	if (error)
	{
		Log::E("Could not read from file.");
		throw FileIOException("Could not read from file.");
	}
}

bool FileSubStream::CopyTo(Stream* stream, size_t size)
{
	CheckValid();

	if (stream == 0)
	{
		return false;
	}
	
	if (GetPosition() + size > GetSize())
	{
		Log::E("The given size goes beyond the end of file.");
		return false;
	}

	const int chunksize = 1024*1024;

	char* buff = new char[chunksize];

	size_t totReadBytes = 0;

	while (size == 0 || totReadBytes < size)
	{
		int readBytes;
		readBytes = fread(buff, 1, chunksize, mFile->GetHandle());

		if (readBytes < chunksize)
		{
			bool returnVal;

			if (feof(mFile->GetHandle()))
			{
				if (readBytes != 0)
				{
					stream->WriteDataBlock(buff, readBytes);
				}

				returnVal = true;
			}
			else
			{
				returnVal = false;
			}

			delete[] buff;
			return returnVal;
		}

		stream->WriteDataBlock(buff, readBytes);
		totReadBytes += readBytes;
	}

	delete [] buff;
	return true;
}

bool FileSubStream::EndOfStream() const
{
	return GetPosition() - mSubPosition == mSubLength;
}

void FileSubStream::WriteByte(uchar /*val*/)
{
	Log::E("Write operation on FileSubStream not supported.");
	throw FileIOException("Write operation on FileSubStream not supported.");
}

void FileSubStream::WriteDataBlock(const void* /*buffer*/, size_t /*size*/)
{
	Log::E("Write operation on FileSubStream not supported.");
	throw FileIOException("Write operation on FileSubStream not supported.");
}

void FileSubStream::Flush()
{
	CheckValid();
	fflush(mFile->GetHandle());
}

void FileSubStream::Seek(int newPos, SeekMode mode)
{
	CheckValid();
	//flush(); <<<---?
	//TODO: Add a check on file boundaries
	fseek(mFile->GetHandle(), newPos + mSubPosition, (mode==SM_SeekFromBegin)?SEEK_SET:(mode==SM_SeekFromCurrent)?SEEK_CUR:SEEK_END);
}

size_t FileSubStream::GetPosition() const
{
	CheckValid();
	return ftell(mFile->GetHandle()) - mSubPosition;
}

size_t FileSubStream::GetSize() const
{
	return mSubLength;
}

bool FileSubStream::CanRead() const
{
	CheckValid();
	return Stream::CanRead();
}

bool FileSubStream::CanWrite() const
{
	return false;
}

bool FileSubStream::CanSeek() const
{
	return true;
}

}
