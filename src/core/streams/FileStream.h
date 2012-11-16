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

#include "Stream.h"
#include "Exceptions.h"
#include "File.h"
#include "Str.h"

namespace crown
{

/**
	File stream.
	Read/write access to files on disk.
*/
class FileStream: public Stream
{

public:

					/**
						Constructor.
					@param filename
						The name of the file to access
					@param openMode
						The mode of access
					*/
					FileStream(StreamOpenMode openMode, const Str& filename);
					/**
						Destructor
					*/
	virtual			~FileStream();
					/** @copydoc Stream::Seek() */
	void			Seek(int32_t newPos, SeekMode mode);
					/** @copydoc Stream::ReadByte() */
	uint8_t			ReadByte();
					/** @copydoc Stream::ReadDataBlock() */
	void			ReadDataBlock(void* buffer, size_t size);
					/** @copydoc Stream::CopyTo() */
	bool			CopyTo(Stream* stream, size_t size = 0);
					/** @copydoc Stream::WriteByte() */
	void			WriteByte(uint8_t val);
					/** @copydoc Stream::WriteDataBlock() */
	void			WriteDataBlock(const void* buffer, size_t size);
					/** @copydoc Stream::Flush() */
	void			Flush();
					/** @copydoc Stream::EndOfStream() */
	bool			EndOfStream() const;
					/** @copydoc Stream::IsValid() */
	bool			IsValid() const
					{
						if (!mFile)
						{
							return false;
						}

						return mFile->IsValid();
					}

					/** @copydoc Stream::GetSize() */
	size_t			GetSize() const;
					/** @copydoc Stream::GetPosition() */
	size_t			GetPosition() const;
					/** @copydoc Stream::CanRead() */
	bool			CanRead() const;
					/** @copydoc Stream::CanWrite() */
	bool			CanWrite() const;
					/** @copydoc Stream::CanSeek() */
	bool			CanSeek() const;

protected:

	File*			mFile;
	bool			mLastWasRead;

	inline void CheckValid() const
	{
		if (!mFile)
		{
			throw InvalidOperationException("Can't operate on an invalid FileStream");
		}
	}
};

}

