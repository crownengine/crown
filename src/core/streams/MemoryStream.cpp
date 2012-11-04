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

#include <stdlib.h>
#include <stdio.h>
#include "MemoryStream.h"
#include "MathUtils.h"
#include "Log.h"
#include "Types.h"

namespace Crown
{

MemoryBuffer::MemoryBuffer()
{
}

MemoryBuffer::~MemoryBuffer()
{
}

DynamicMemoryBuffer::DynamicMemoryBuffer(size_t initialCapacity)
{
	mBuff = 0;
	Allocate(initialCapacity);
}

DynamicMemoryBuffer::~DynamicMemoryBuffer()
{
	Release();
}

void DynamicMemoryBuffer::Allocate(size_t capacity)
{
	Release();
	mBuff = new uchar[capacity];
	this->mCapacity = capacity;
	mSize = 0;
}

void DynamicMemoryBuffer::Release()
{
	if (mBuff != 0)
	{
		delete[] mBuff;
		mBuff = 0;
	}
}

void DynamicMemoryBuffer::CheckSpace(size_t offset, size_t size)
{
	if (offset + size > mCapacity)
	{
		mCapacity = (size_t) ((offset + size) * 1.2f);
		mBuff = (uchar*) realloc(mBuff, mCapacity);
	}
}

void DynamicMemoryBuffer::Write(uchar* src, size_t offset, size_t size)
{
	CheckSpace(offset, size);

	for (size_t i=0; i<size; i++)
	{
		mBuff[offset+i] = src[i];
	}

	//If the writing goes beyond the end of buffer
	if (offset + size > this->mSize)
	{
		this->mSize = offset + size;
	}
}

MemoryStream::MemoryStream(MemoryBuffer* f, StreamOpenMode openMode) :
	Stream(openMode)
{
	mMem = f;
	mMemOffset = 0;
}

MemoryStream::~MemoryStream()
{
	if (mMem)
	{
		delete mMem;
		mMem = 0;
	}
}

void MemoryStream::Seek(int newPos, SeekMode mode)
{
	CheckValid();
	
	switch (mode)
	{
		case SM_SeekFromBegin:
			mMemOffset = newPos;
			break;
		case SM_SeekFromCurrent:
			mMemOffset += newPos;
			break;
		case SM_SeekFromEnd:
			mMemOffset = mMem->GetSize()-1;
			break;
	}

	//Allow seek to mMem->getSize() position, that means end of stream, reading not allowed but you can write if it's dynamic
	if (mMemOffset > mMem->GetSize())
	{
		Log::E("Seek beyond the end of stream.");
		throw InvalidOperationException("Seek beyond the end of stream.");
	}
}

uchar MemoryStream::ReadByte()
{
	CheckValid();

	if (mMemOffset >= mMem->GetSize())
	{
		Log::E("Trying to read beyond the end of stream.");
		throw InvalidOperationException("Trying to read beyond the end of stream.");
	}

	return mMem->GetData()[mMemOffset++];
}

void MemoryStream::ReadDataBlock(void* buffer, size_t size)
{
	CheckValid();
	uchar* src = mMem->GetData();
	uchar* dest = (uchar*) buffer;

	if (mMemOffset + size > mMem->GetSize())
	{
		Log::E("Trying to read beyond the end of stream.");
		throw InvalidOperationException("Trying to read beyond the end of stream.");
	}

	for (size_t i = 0; i < size; i++)
	{
		dest[i] = src[mMemOffset+i];
	}

	mMemOffset += size;
}

bool MemoryStream::CopyTo(Stream* stream, size_t size)
{
	CheckValid();
	stream->WriteDataBlock(&(mMem->GetData()[mMemOffset]), math::min(mMem->GetSize()-mMemOffset, size));
	return true;
}

void MemoryStream::WriteByte(uchar val)
{
	CheckValid();
	mMem->Write(&val, mMemOffset, 1);
	mMemOffset++;
}

void MemoryStream::WriteDataBlock(const void* buffer, size_t size)
{
	CheckValid();
	mMem->Write((uchar*)buffer, mMemOffset, size);
	mMemOffset += size;
}

void MemoryStream::Flush()
{
	return;
}

void MemoryStream::Dump()
{
	uchar* buff = mMem->GetData();

	for (size_t i=0; i<mMem->GetSize(); i++)
	{
		printf("%3i ", buff[i]);
	}
}

}
