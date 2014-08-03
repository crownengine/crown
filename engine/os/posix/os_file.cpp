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

#include <stdio.h>

#include "assert.h"
#include "os.h"
#include "os_file.h"

namespace crown
{

//-----------------------------------------------------------------------------
OsFile::OsFile(const char* path, FileOpenMode mode) :
	m_file_handle(NULL)
{
	m_file_handle = fopen(path, (mode == FOM_READ) ? "rb" : "wb");
	CE_ASSERT(m_file_handle != NULL, "Unable to open file: %s", path);

	m_mode = mode;
}

//-----------------------------------------------------------------------------
OsFile::~OsFile()
{
	close();
}

//-----------------------------------------------------------------------------
void OsFile::close()
{
	if (m_file_handle != NULL)
	{
		fclose(m_file_handle);
		m_file_handle = NULL;
	}
}

//-----------------------------------------------------------------------------
bool OsFile::is_open() const
{
	return m_file_handle != NULL;
}

//-----------------------------------------------------------------------------
FileOpenMode OsFile::mode()
{
	return m_mode;
}

//-----------------------------------------------------------------------------
size_t OsFile::size() const
{
	size_t pos = position();

	int fseek_result = fseek(m_file_handle, 0, SEEK_END);
	CE_ASSERT(fseek_result == 0, "Failed to seek");

	size_t size = position();

	fseek_result = fseek(m_file_handle, (long) pos, SEEK_SET);
	CE_ASSERT(fseek_result == 0, "Failed to seek");
	CE_UNUSED(fseek_result);

	return size;
}

//-----------------------------------------------------------------------------
size_t OsFile::read(void* data, size_t size)
{
	CE_ASSERT(data != NULL, "Data must be != NULL");

	return fread(data, 1, size, m_file_handle);
}

//-----------------------------------------------------------------------------
size_t OsFile::write(const void* data, size_t size)
{
	CE_ASSERT(data != NULL, "Data must be != NULL");

	return fwrite(data, 1, size, m_file_handle);
}

//-----------------------------------------------------------------------------
void OsFile::seek(size_t position)
{
	int fseek_result = fseek(m_file_handle, (long) position, SEEK_SET);
	CE_ASSERT(fseek_result == 0, "Failed to seek");
	CE_UNUSED(fseek_result);
}

//-----------------------------------------------------------------------------
void OsFile::seek_to_end()
{
	int fseek_result = fseek(m_file_handle, 0, SEEK_END);
	CE_ASSERT(fseek_result == 0, "Failed to seek");
	CE_UNUSED(fseek_result);
}

//-----------------------------------------------------------------------------
void OsFile::skip(size_t bytes)
{
	int fseek_result = fseek(m_file_handle, bytes, SEEK_CUR);
	CE_ASSERT(fseek_result == 0, "Failed to seek");
	CE_UNUSED(fseek_result);
}

//-----------------------------------------------------------------------------
size_t OsFile::position() const
{
	return (size_t) ftell(m_file_handle);
}

//-----------------------------------------------------------------------------
bool OsFile::eof() const
{
	return feof(m_file_handle) != 0;
}

} // namespace crown

