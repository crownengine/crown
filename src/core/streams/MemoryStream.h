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

#pragma once

#include "Types.h"
#include "Stream.h"
#include "Exceptions.h"

namespace crown
{

class MemoryBuffer
{

public:

						MemoryBuffer();
	virtual				~MemoryBuffer();

	virtual void		Release() = 0;
	virtual void		Allocate(size_t size) = 0;

	virtual bool		IsValid() = 0;
	virtual size_t		GetSize() = 0;
	virtual uint8_t*		GetData() = 0;

	virtual void		Write(uint8_t* src, size_t offset, size_t size) = 0;
};

class DynamicMemoryBuffer: public MemoryBuffer
{

public:

					DynamicMemoryBuffer(size_t initialCapacity);
	virtual			~DynamicMemoryBuffer();

	void			Release();
	void			Allocate(size_t capacity);

	inline bool		IsValid() { return mBuff != 0; }

	void			CheckSpace(size_t offset, size_t space);
	void			Write(uint8_t* src, size_t offset, size_t size);

	inline size_t	GetSize() { return mSize; }
	inline size_t	GetCapacity() { return mCapacity; }

	inline uint8_t*	GetData() { return mBuff; }

protected:

	uint8_t*			mBuff;
	size_t			mCapacity;
	size_t			mSize;
};

/**
	Memory stream.

	Access memory buffers.
*/
class MemoryStream: public Stream
{

public:

						MemoryStream(MemoryBuffer* f, StreamOpenMode openMode);
	virtual				~MemoryStream();

	void				Seek(int newPos, SeekMode mode);

	uint8_t				ReadByte();
	void				ReadDataBlock(void* buffer, size_t size);
	bool				CopyTo(Stream* stream, size_t size = 0);

	void				WriteByte(uint8_t val);
	void				WriteDataBlock(const void* buffer, size_t size);
	void				Flush();

	bool				EndOfStream() const { return GetSize() == mMemOffset; }
	bool				IsValid() const { CheckValid(); return mMem->IsValid(); }

	size_t				GetSize() const { CheckValid(); return mMem->GetSize(); }
	size_t				GetPosition() const { return mMemOffset; }

	bool				CanRead() const { return Stream::CanRead(); }
	bool				CanWrite() const { return Stream::CanWrite(); }
	bool				CanSeek() const { return true; }

	void				Dump();

protected:

	MemoryBuffer*		mMem;
	size_t				mMemOffset;

	inline void			CheckValid() const
						{
							if (!mMem)
							{
								throw InvalidOperationException("Can't operate on an invalid MemoryStream");
							}
						}
};

}
