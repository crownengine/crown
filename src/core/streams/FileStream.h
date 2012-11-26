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
#include "File.h"
#include <cassert>

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
					FileStream(StreamOpenMode mode, const char* filename);
					/**
						Destructor
					*/
	virtual			~FileStream();

					/** @copydoc Stream::seek() */
	void			seek(size_t position);
					/** @copydoc Stream::seek_to_end() */
	void			seek_to_end();
					/** @copydoc Stream::skip() */
	void			skip(size_t bytes);
					/** @copydoc Stream::ReadByte() */
	uint8_t			read_byte();
					/** @copydoc Stream::ReadDataBlock() */
	void			read(void* buffer, size_t size);
					/** @copydoc Stream::WriteByte() */
	void			write_byte(uint8_t val);
					/** @copydoc Stream::WriteDataBlock() */
	void			write(const void* buffer, size_t size);
					/** @copydoc Stream::CopyTo() */
	bool			copy_to(Stream* stream, size_t size = 0);
					/** @copydoc Stream::Flush() */
	void			flush();
					/** @copydoc Stream::EndOfStream() */
	bool			end_of_stream() const;
					/** @copydoc Stream::is_valid() */
	bool			is_valid() const;
					/** @copydoc Stream::GetSize() */
	size_t			size() const;
					/** @copydoc Stream::GetPosition() */
	size_t			position() const;
					/** @copydoc Stream::CanRead() */
	bool			can_read() const;
					/** @copydoc Stream::CanWrite() */
	bool			can_write() const;
					/** @copydoc Stream::CanSeek() */
	bool			can_seek() const;

protected:

	File*			m_file;
	bool			m_last_was_read;

	inline void		check_valid() const
					{
						assert(m_file != NULL);
					}
};

} // namespace crown

