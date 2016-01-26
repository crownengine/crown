/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "filesystem_types.h" // FileOpenMode
#include "types.h"
#include "error.h"
#include "macros.h"
#include "platform.h"

#if CROWN_PLATFORM_POSIX
	#include <stdio.h>
	#include <errno.h>
#elif CROWN_PLATFORM_WINDOWS
	#include "tchar.h"
	#include "win_headers.h"
#endif

namespace crown
{
/// Standard C file wrapper
class OsFile
{
#if CROWN_PLATFORM_POSIX
	FILE* _file;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE _file;
	bool _eof;
#endif

public:

	/// Opens the file located at @a path with the given @a mode.
	OsFile()
#if CROWN_PLATFORM_POSIX
		: _file(NULL)
#elif CROWN_PLATFORM_WINDOWS
		: _file(INVALID_HANDLE_VALUE)
		, _eof(false)
#endif
	{
	}

	~OsFile()
	{
		close();
	}

	void open(const char* path, FileOpenMode::Enum mode)
	{
#if CROWN_PLATFORM_POSIX
		_file = fopen(path, (mode == FileOpenMode::READ) ? "rb" : "wb");
		CE_ASSERT(_file != NULL, "fopen: errno = %d, path = '%s'", errno, path);
#elif CROWN_PLATFORM_WINDOWS
		_file = CreateFile(path
			, (mode == FileOpenMode::READ) ? GENERIC_READ : GENERIC_WRITE
			, 0
			, NULL
			, OPEN_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL
			);
		CE_ASSERT(_file != INVALID_HANDLE_VALUE
			, "CreateFile: GetLastError = %d, path = '%s'"
			, GetLastError()
			, path
			);
#endif
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
			_file = INVALID_HANDLE_VALUE;
		}
#endif
	}

	bool is_open() const
	{
#if CROWN_PLATFORM_POSIX
		return _file != NULL;
#elif CROWN_PLATFORM_WINDOWS
		return _file != INVALID_HANDLE_VALUE;
#endif
	}

	/// Return the size of the file in bytes.
	uint32_t size() const
	{
#if CROWN_PLATFORM_POSIX
		size_t pos = position();

		int err = fseek(_file, 0, SEEK_END);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);

		size_t size = position();

		err = fseek(_file, (long)pos, SEEK_SET);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
		CE_UNUSED(err);

		return (uint32_t)size;
#elif CROWN_PLATFORM_WINDOWS
		return GetFileSize(_file, NULL);
#endif
	}

	/// Reads @a size bytes from the file and stores it into @a data.
	/// Returns the number of bytes read.
	uint32_t read(void* data, uint32_t size)
	{
		CE_ASSERT(data != NULL, "Data must be != NULL");
#if CROWN_PLATFORM_POSIX
		size_t bytes_read = fread(data, 1, size, _file);
		CE_ASSERT(ferror(_file) == 0, "fread error");
		return (uint32_t)bytes_read;
#elif CROWN_PLATFORM_WINDOWS
		DWORD bytes_read;
		BOOL result = ReadFile(_file, data, size, &bytes_read, NULL);
		CE_ASSERT(result == TRUE, "ReadFile: GetLastError = %d", GetLastError());

		if (result && bytes_read == 0)
			_eof = true;

		return bytes_read;
#endif
	}

	/// Writes @a size bytes of data stored in @a data and returns the
	/// number of bytes written.
	uint32_t write(const void* data, uint32_t size)
	{
		CE_ASSERT(data != NULL, "Data must be != NULL");
#if CROWN_PLATFORM_POSIX
		size_t bytes_written = fwrite(data, 1, size, _file);
		CE_ASSERT(ferror(_file) == 0, "fwrite error");
		return (uint32_t)bytes_written;
#elif CROWN_PLATFORM_WINDOWS
		DWORD bytes_written;
		WriteFile(_file, data, size, &bytes_written, NULL);
		CE_ASSERT(size == bytes_written, "WriteFile: GetLastError = %d", GetLastError());
		return bytes_written;
#endif
	}

	void flush()
	{
#if CROWN_PLATFORM_POSIX
		int err = fflush(_file);
		CE_ASSERT(err == 0, "fflush: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		BOOL err = FlushFileBuffers(_file);
		CE_ASSERT(err != 0, "FlushFileBuffers: GetLastError = %d", GetLastError());
#endif
		CE_UNUSED(err);
	}

	/// Moves the file pointer to the given @a position.
	void seek(uint32_t position)
	{
#if CROWN_PLATFORM_POSIX
		int err = fseek(_file, (long)position, SEEK_SET);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, position, NULL, FILE_BEGIN);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER, "SetFilePointer: GetLastError = %d", GetLastError());
#endif
		CE_UNUSED(err);
	}

	/// Moves the file pointer to the end of the file.
	void seek_to_end()
	{
#if CROWN_PLATFORM_POSIX
		int err = fseek(_file, 0, SEEK_END);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, 0, NULL, FILE_END);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER, "SetFilePointer: GetLastError = %d", GetLastError());
#endif
		CE_UNUSED(err);
	}

	/// Moves the file pointer @a bytes bytes ahead the current
	/// file pointer position.
	void skip(uint32_t bytes)
	{
#if CROWN_PLATFORM_POSIX
		int err = fseek(_file, bytes, SEEK_CUR);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, bytes, NULL, FILE_CURRENT);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER, "SetFilePointer: GetLastError = %d", GetLastError());
#endif
		CE_UNUSED(err);
	}

	/// Returns the position of the file pointer from the
	/// start of the file in bytes.
	uint32_t position() const
	{
#if CROWN_PLATFORM_POSIX
		long pos = ftell(_file);
		CE_ASSERT(pos != -1, "ftell: errno = %d", errno);
		return (uint32_t)pos;
#elif CROWN_PLATFORM_WINDOWS
		DWORD pos = SetFilePointer(_file, 0, NULL, FILE_CURRENT);
		CE_ASSERT(pos != INVALID_SET_FILE_POINTER, "SetFilePointer: GetLastError = %d", GetLastError());
		return (uint32_t)pos;
#endif
		CE_UNUSED(pos);
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
};

} // namespace crown
