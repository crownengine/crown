/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "file.h" // FileOpenMode
#include "types.h"
#include "assert.h"
#include "macros.h"
#include "config.h"

#if CROWN_PLATFORM_POSIX
	#include <cstdio>
#elif CROWN_PLATFORM_WINDOWS
	#include "tchar.h"
	#include "win_headers.h"
#endif

namespace crown
{

/// Standard C file wrapper
class OsFile
{
public:

	/// Opens the file located at @a path with the given @a mode.
	OsFile(const char* path, FileOpenMode mode)
#if CROWN_PLATFORM_WINDOWS
		: _eof(false)
#endif
	{
#if CROWN_PLATFORM_POSIX
		_file = fopen(path, (mode == FOM_READ) ? "rb" : "wb");
		CE_ASSERT(_file != NULL, "Unable to open file: %s", path);
#elif CROWN_PLATFORM_WINDOWS
		_file = CreateFile(path,
			mode == FOM_READ ? GENERIC_READ : GENERIC_WRITE,
			0,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		CE_ASSERT(_file != INVALID_HANDLE_VALUE, "Unable to open file: %s", path);
#endif
	}

	~OsFile()
	{
		close();
	}

	/// Closes the file.
	void close()
	{
#if CROWN_PLATFORM_POSIX
		if (_file != NULL)
		{
			fclose(_file);
			_file = NULL;
		}
#elif CROWN_PLATFORM_WINDOWS
		if (is_open())
		{
			CloseHandle(_file);
			_file = NULL;
		}
#endif
	}

	bool is_open() const
	{
		return _file != NULL;
	}

	/// Return the size of the file in bytes.
	size_t size() const
	{
#if CROWN_PLATFORM_POSIX
		size_t pos = position();

		int fseek_result = fseek(_file, 0, SEEK_END);
		CE_ASSERT(fseek_result == 0, "Failed to seek");

		size_t size = position();

		fseek_result = fseek(_file, (long) pos, SEEK_SET);
		CE_ASSERT(fseek_result == 0, "Failed to seek");
		CE_UNUSED(fseek_result);

		return size;
#elif CROWN_PLATFORM_WINDOWS
		return GetFileSize(_file, NULL);
#endif		
	}

	/// Reads @a size bytes from the file and stores it into @a data.
	/// Returns the number of bytes read.
	size_t read(void* data, size_t size)
	{
		CE_ASSERT(data != NULL, "Data must be != NULL");
#if CROWN_PLATFORM_POSIX
		return fread(data, 1, size, _file);
#elif CROWN_PLATFORM_WINDOWS
		DWORD bytes_read;

		BOOL result = ReadFile(_file, data, size, &bytes_read, NULL);

		CE_ASSERT(result == TRUE, "Unable to read from file");

		if (result && bytes_read == 0)
		{
			_eof = true;
		}

		return bytes_read;
#endif
	}

	/// Writes @a size bytes of data stored in @a data and returns the
	/// number of bytes written.
	size_t write(const void* data, size_t size)
	{
		CE_ASSERT(data != NULL, "Data must be != NULL");
#if CROWN_PLATFORM_POSIX
		return fwrite(data, 1, size, _file);
#elif CROWN_PLATFORM_WINDOWS
		DWORD bytes_written;
		WriteFile(_file, data, size, &bytes_written, NULL);
		CE_ASSERT(size == bytes_written, "Cannot read from file\n");
		return size;
#endif
	}

	/// Moves the file pointer to the given @a position.
	void seek(size_t position)
	{
#if CROWN_PLATFORM_POSIX
		int err = fseek(_file, (long) position, SEEK_SET);
		CE_ASSERT(err == 0, "Failed to seek");
#elif CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, position, NULL, FILE_BEGIN);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER, "Failed to seek");
#endif		
		CE_UNUSED(err);
	}

	/// Moves the file pointer to the end of the file.
	void seek_to_end()
	{
#if CROWN_PLATFORM_POSIX
		int fseek_result = fseek(_file, 0, SEEK_END);
		CE_ASSERT(fseek_result == 0, "Failed to seek");
		CE_UNUSED(fseek_result);
#elif CROWN_PLATFORM_WINDOWS
		DWORD seek_result = SetFilePointer(_file, 0, NULL, FILE_END);
		CE_ASSERT(seek_result != INVALID_SET_FILE_POINTER, "Failed to seek to end");
#endif
	}

	/// Moves the file pointer @a bytes bytes ahead the current
	/// file pointer position.
	void skip(size_t bytes)
	{
#if CROWN_PLATFORM_POSIX
		int fseek_result = fseek(_file, bytes, SEEK_CUR);
		CE_ASSERT(fseek_result == 0, "Failed to seek");
		CE_UNUSED(fseek_result);
#elif CROWN_PLATFORM_WINDOWS
		DWORD seek_result = SetFilePointer(_file, bytes, NULL, FILE_CURRENT);

		CE_ASSERT(seek_result != INVALID_SET_FILE_POINTER, "Failed to skip");
#endif
	}

	/// Returns the position of the file pointer from the
	/// start of the file in bytes.
	size_t position() const
	{
#if CROWN_PLATFORM_POSIX		
		return (size_t) ftell(_file);
#elif CROWN_PLATFORM_WINDOWS
		DWORD position = SetFilePointer(_file, 0, NULL, FILE_CURRENT);

		CE_ASSERT(position != INVALID_SET_FILE_POINTER, "Failed to get position");

		return position;
#endif
	}

	/// Returns whether the file pointer is at the end of the file.
	bool eof() const
	{
#if CROWN_PLATFORM_POSIX
		return feof(_file) != 0;
#elif CROWN_PLATFORM_WINDOWS
		return _eof;
#endif		
	}

private:

#if CROWN_PLATFORM_POSIX
	FILE* _file;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE _file;
	bool _eof;
#endif
};

} // namespace crown
