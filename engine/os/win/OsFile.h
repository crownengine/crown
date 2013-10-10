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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <cstdio>
#include <windows.h>
#include "Types.h"
#include "File.h"

namespace crown
{

/// Standard C file wrapper
class OsFile
{
public:

	/// Opens the file located at @a path with the given @a mode.
							OsFile(const char* path, FileOpenMode mode);
							~OsFile();

	/// Closes the file.
	void					close();

	bool					is_open() const;

	/// Return the size of the file in bytes.
	size_t					size() const;

	/// Returs the mode used to open the file.
	FileOpenMode			mode();

	/// Reads @a size bytes from the file and stores it into @a data.
	/// Returns the number of bytes read.
	size_t					read(void* data, size_t size);

	/// Writes @a size bytes of data stored in @a data and returns the
	/// number of bytes written.
	size_t					write(const void* data, size_t size);

	/// Moves the file pointer to the given @a position.
	void					seek(size_t position);

	/// Moves the file pointer to the end of the file.
	void					seek_to_end();

	/// Moves the file pointer @a bytes bytes ahead the current
	/// file pointer position.
	void					skip(size_t bytes);

	/// Returns the position of the file pointer from the
	/// start of the file in bytes.
	size_t					position() const;

	/// Returns whether the file pointer is at the end of the file.
	bool					eof() const;

private:

	HANDLE					m_file_handle;
	FileOpenMode			m_mode;
	bool					m_eof;
};

} // namespace crown

