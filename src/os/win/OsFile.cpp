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

#include "OsFile.h"


namespace crown
{

OsFile::OsFile(const char* path, FileOpenMode mode)
{
	m_file_handle = CreateFile(path,
							   mode == FOM_READ ? GENERIC_READ : GENERIC_WRITE,
							   FILE_SHARE_READ | FILE_SHARE_WRITE,
							   NULL,
							   OPEN_EXISTING,
        					   FILE_ATTRIBUTE_NORMAL,
        					   NULL);

	CE_ASSERT(m_file_handle != NULL, "Unable to open file: %s", path);

	m_mode = mode;
}

OsFile::~OsFile()
{
	close();
}

void OsFile::close()
{
	if (!CloseHandle(m_file_handle))
	{
		CE_ASSERT(false, "Unable to close file\n");
	}
}

bool OsFile::is_open() const
{
	return m_file_handle != NULL;
}

size_t OsFile::size() const
{
	LPDWORD size;

	GetFileSize(m_file_handle, size);

	return size;
}

FileOpenMode OsFile::mode()
{
	return m_mode;
}

size_t OsFile::read(void* data, size_t size)
{
	bool read = ReadFile(m_file_handle, data, size, NULL, NULL))
	
	CE_ASSERT(read, "Cannot read from file\n");

	return size;
}

size_t OsFile::write(const void* data, size_t size)
{
	bool write = WriteFile(m_file_handle, data, size, NULL, NULL))
	
	CE_ASSERT(write, "Cannot read from file\n");		

	return size;
}

void OsFile::seek(size_t position)
{
	DWORD seek_result = SetFilePointer(m_file_handle, (LONG) position, NULL, FILE_BEGIN);

	CE_ASSERT(seek_result == 0, "Failed to seek");
}

void OsFile::seek_to_end()
{
	DWORD seek_result = SetFilePointer(m_file_handle, 0, NULL, FILE_END);

	CE_ASSERT(seek_result == 0, "Failed to seek");
}

void OsFile::skip(size_t bytes)
{
	DWORD seek_result = SetFilePointer(m_file_handle, (LONG) bytes, NULL, FILE_CURRENT);

	CE_ASSERT(seek_result == 0, "Failed to seek");
}

size_t OsFile::position() const
{
	DWORD position = SetFilePointer(m_file_handle, 0, NULL, FILE_CURRENT);

	return position;
}

bool OsFile::eof() const
{
	return feof(m_file_handle) != 0;
}

} // namespace crown

