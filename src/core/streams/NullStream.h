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

namespace crown
{

/**
	Bit bucket stream.

	Discards all data written to it and provides null data reading from it; plain and simple.
*/
class NullStream: public Stream
{

public:

				/** @copydoc Stream::Stream() */
				NullStream(StreamOpenMode mode) : Stream(mode) {}
				/** @copydoc Stream::~Stream() */
	virtual		~NullStream() {}

				/** @copydoc Stream::Seek() */
	void		seek(int32_t /*position*/, uint8_t /*mode*/) {}
				/**
				@copydoc Stream::ReadByte()
				@note
					Returns always zero
				*/
	uint8_t		read_byte() { return 0; }
				/**
				@copydoc Stream::ReadDataBlock()
				@note
					Fills buffer with zeroes
				*/
	void		read_data_block(void* buffer, size_t size)
				{
					for (size_t i = 0; i < size; i++)
					{
						((uint8_t*)buffer)[i] = 0;
					}
				}
				/**
				@copydoc Stream::CopyTo()
				@note
					Returns always false
				*/
	bool		copy_to(Stream* /*stream*/, size_t /*size = 0*/) { return false; }
				/** @copydoc Stream::WriteByte() */
	void		write_byte(uint8_t /*val*/) {};
				/** @copydoc Stream::WriteDataBlock() */
	void		write_data_block(const void* /*buffer*/, size_t /*size*/) {};
				/** @copydoc Stream::Flush() */
	void		flush() {};
				/**
				@copydoc Stream::IsValid()
				@note
					Returns always true
				*/
	bool		is_valid() { return true; }
				/**
				@copydoc Stream::EndOfStream()
				@note
					Returns always false
				*/
	bool		end_of_stream() { return false; }
				/**
				@copydoc Stream::GetSize()
				@note
					Returns always 0xFFFFFFFF
				*/
	size_t		size() { return ~0; }
				/**
				@copydoc Stream::GetPosition()
				@note
					Returns always zero
				*/
	size_t		position() { return 0; }
				/**
				@copydoc Stream::CanRead()
				@note
					Returns always true
				*/
	bool		can_read() { return true; }
				/**
				@copydoc Stream::CanWrite()
				@note
					Returns always true
				*/
	bool		can_write() { return true; }
				/**
				@copydoc Stream::CanSeek()
				@note
					Returns always true
				*/
	bool		can_seek() { return true; }
};

} // namespace crown

