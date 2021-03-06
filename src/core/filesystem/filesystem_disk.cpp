/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/vector.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/path.h"
#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/strings/dynamic_string.inl"

#if CROWN_PLATFORM_POSIX
	#include <stdio.h>
	#include <errno.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <tchar.h>
	#include <windows.h>
#endif

namespace crown
{
struct FileDisk : public File
{
#if CROWN_PLATFORM_POSIX
	FILE* _file;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE _file;
	bool _eof;
#endif

	/// Opens the file located at @a path with the given @a mode.
	FileDisk()
#if CROWN_PLATFORM_POSIX
		: _file(NULL)
#elif CROWN_PLATFORM_WINDOWS
		: _file(INVALID_HANDLE_VALUE)
		, _eof(false)
#endif
	{
	}

	virtual ~FileDisk()
	{
		close();
	}

	void open(const char* path, FileOpenMode::Enum mode)
	{
#if CROWN_PLATFORM_POSIX
		_file = fopen(path, (mode == FileOpenMode::READ) ? "rb" : "wb");
#elif CROWN_PLATFORM_WINDOWS
		_file = CreateFile(path
			, (mode == FileOpenMode::READ) ? GENERIC_READ : GENERIC_WRITE
			, 0
			, NULL
			, (mode == FileOpenMode::READ) ? OPEN_EXISTING : CREATE_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL
			);
#endif
	}

	void close()
	{
		if (is_open())
		{
#if CROWN_PLATFORM_POSIX
			fclose(_file);
			_file = NULL;
#elif CROWN_PLATFORM_WINDOWS
			CloseHandle(_file);
			_file = INVALID_HANDLE_VALUE;
#endif
		}
	}

	bool is_open()
	{
#if CROWN_PLATFORM_POSIX
		return _file != NULL;
#elif CROWN_PLATFORM_WINDOWS
		return _file != INVALID_HANDLE_VALUE;
#endif
	}

	u32 size()
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_POSIX
		Stat stat;
		os::stat(stat, fileno(_file));
		return (u32)stat.size;
#elif CROWN_PLATFORM_WINDOWS
		return GetFileSize(_file, NULL);
#endif
	}

	u32 position()
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_POSIX
		long pos = ftell(_file);
		CE_ASSERT(pos != -1, "ftell: errno = %d", errno);
		return (u32)pos;
#elif CROWN_PLATFORM_WINDOWS
		DWORD pos = SetFilePointer(_file, 0, NULL, FILE_CURRENT);
		CE_ASSERT(pos != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
		return (u32)pos;
#endif
	}

	bool end_of_file()
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_POSIX
		return feof(_file) != 0;
#elif CROWN_PLATFORM_WINDOWS
		return _eof;
#endif
	}

	void seek(u32 position)
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_POSIX
		int err = fseek(_file, (long)position, SEEK_SET);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, position, NULL, FILE_BEGIN);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		CE_UNUSED(err);
	}

	void seek_to_end()
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_POSIX
		int err = fseek(_file, 0, SEEK_END);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, 0, NULL, FILE_END);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		CE_UNUSED(err);
	}

	void skip(u32 bytes)
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_POSIX
		int err = fseek(_file, bytes, SEEK_CUR);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, bytes, NULL, FILE_CURRENT);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#endif
		CE_UNUSED(err);
	}

	u32 read(void* data, u32 size)
	{
		CE_ASSERT(is_open(), "File is not open");
		CE_ASSERT(data != NULL, "Data must be != NULL");
#if CROWN_PLATFORM_POSIX
		size_t bytes_read = fread(data, 1, size, _file);
		CE_ASSERT(ferror(_file) == 0, "fread error");
		return (u32)bytes_read;
#elif CROWN_PLATFORM_WINDOWS
		DWORD bytes_read;
		BOOL err = ReadFile(_file, data, size, &bytes_read, NULL);
		CE_ASSERT(err == TRUE, "ReadFile: GetLastError = %d", GetLastError());
		_eof = err && bytes_read == 0;
		return bytes_read;
#endif
	}

	u32 write(const void* data, u32 size)
	{
		CE_ASSERT(is_open(), "File is not open");
		CE_ASSERT(data != NULL, "Data must be != NULL");
#if CROWN_PLATFORM_POSIX
		size_t bytes_written = fwrite(data, 1, size, _file);
		CE_ASSERT(ferror(_file) == 0, "fwrite error");
		return (u32)bytes_written;
#elif CROWN_PLATFORM_WINDOWS
		DWORD bytes_written;
		WriteFile(_file, data, size, &bytes_written, NULL);
		CE_ASSERT(size == bytes_written
			, "WriteFile: GetLastError = %d"
			, GetLastError()
			);
		return bytes_written;
#endif
	}

	void flush()
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_POSIX
		int err = fflush(_file);
		CE_ASSERT(err == 0, "fflush: errno = %d", errno);
#elif CROWN_PLATFORM_WINDOWS
		BOOL err = FlushFileBuffers(_file);
		CE_ASSERT(err != 0
			, "FlushFileBuffers: GetLastError = %d"
			, GetLastError()
			);
#endif
		CE_UNUSED(err);
	}
};

FilesystemDisk::FilesystemDisk(Allocator& a)
	: _allocator(&a)
	, _prefix(a)
{
}

void FilesystemDisk::set_prefix(const char* prefix)
{
	_prefix.set(prefix, strlen32(prefix));
}

File* FilesystemDisk::open(const char* path, FileOpenMode::Enum mode)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	FileDisk* file = CE_NEW(*_allocator, FileDisk)();
	file->open(abs_path.c_str(), mode);
	return file;
}

void FilesystemDisk::close(File& file)
{
	CE_DELETE(*_allocator, &file);
}

Stat FilesystemDisk::stat(const char* path)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	Stat info;
	os::stat(info, abs_path.c_str());
	return info;
}

bool FilesystemDisk::exists(const char* path)
{
	return stat(path).file_type != Stat::NO_ENTRY;
}

bool FilesystemDisk::is_directory(const char* path)
{
	return stat(path).file_type == Stat::DIRECTORY;
}

bool FilesystemDisk::is_file(const char* path)
{
	return stat(path).file_type == Stat::REGULAR;
}

u64 FilesystemDisk::last_modified_time(const char* path)
{
	return stat(path).mtime;
}

CreateResult FilesystemDisk::create_directory(const char* path)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	return os::create_directory(abs_path.c_str());
}

DeleteResult FilesystemDisk::delete_directory(const char* path)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	return os::delete_directory(abs_path.c_str());
}

DeleteResult FilesystemDisk::delete_file(const char* path)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	return os::delete_file(abs_path.c_str());
}

void FilesystemDisk::list_files(const char* path, Vector<DynamicString>& files)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	os::list_files(abs_path.c_str(), files);
}

void FilesystemDisk::absolute_path(DynamicString& os_path, const char* path)
{
	if (path::is_absolute(path))
	{
		os_path = path;
		return;
	}

	TempAllocator1024 ta;
	DynamicString str(ta);
	path::join(str, _prefix.c_str(), path);
	path::reduce(os_path, str.c_str());
}

} // namespace crown
