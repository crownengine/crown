/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include "Assert.h"
#include "OsFile.h"
#include "File.h"

namespace crown
{

/// Provides common facilities to access files on disk.
class DiskFile: public File
{
public:

	/// Opens @a filename with specified @a mode
					DiskFile(FileOpenMode mode, const char* filename);
	virtual			~DiskFile();

	/// @a copydoc File::seek() 
	void			seek(size_t position);

	/// @a copydoc File::seek_to_end() 
	void			seek_to_end();

	/// @a copydoc File::skip() 
	void			skip(size_t bytes);

	/// @a copydoc File::read() 
	void			read(void* buffer, size_t size);

	/// @a copydoc File::write() 
	void			write(const void* buffer, size_t size);

	/// @a copydoc File::copy_to() 
	bool			copy_to(File& file, size_t size = 0);

	/// @a copydoc File::flush() 
	void			flush();

	/// @a copydoc File::end_of_file() 
	bool			end_of_file() const;

	/// @a copydoc File::is_valid() 
	bool			is_valid() const;

	/// @a copydoc File::size() 
	size_t			size() const;

	/// @a copydoc File::position() 
	size_t			position() const;

	/// @a copydoc File::can_read() 
	bool			can_read() const;

	/// @a copydoc File::can_write() 
	bool			can_write() const;

	/// @a copydoc File::can_seek() 
	bool			can_seek() const;

protected:

	OsFile			m_file;
	bool			m_last_was_read;

protected:

	inline void		check_valid() const
	{
		CE_ASSERT(m_file.is_open(), "File is not open");
	}
};

} // namespace crown

