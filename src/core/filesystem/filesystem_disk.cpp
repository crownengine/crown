/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/vector.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/path.h"
#include "core/guid.h"
#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/strings/dynamic_string.inl"

#if CROWN_PLATFORM_WINDOWS
	#include <tchar.h>
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#else
	#include <stdio.h>
	#include <errno.h>
	#include <unistd.h> // fsync
#endif

namespace crown
{
struct FileDisk : public File
{
#if CROWN_PLATFORM_WINDOWS
	HANDLE _file;
	bool _eof;
#else
	FILE *_file;
#endif

	/// Opens the file located at @a path with the given @a mode.
	FileDisk()
#if CROWN_PLATFORM_WINDOWS
		: _file(INVALID_HANDLE_VALUE)
		, _eof(false)
#else
		: _file(NULL)
#endif
	{
	}

	virtual ~FileDisk()
	{
		close();
	}

	void open(const char *path, FileOpenMode::Enum mode) override
	{
#if CROWN_PLATFORM_WINDOWS
		_file = CreateFile(path
			, (mode == FileOpenMode::READ) ? GENERIC_READ : GENERIC_WRITE
			, (mode == FileOpenMode::READ) ? FILE_SHARE_READ : 0 /* Exclusive write access. */
			, NULL
			, (mode == FileOpenMode::READ) ? OPEN_EXISTING : CREATE_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL
			);
#else
		_file = fopen(path, (mode == FileOpenMode::READ) ? "rb" : "wb");
#endif
	}

	void close() override
	{
		if (is_open()) {
#if CROWN_PLATFORM_WINDOWS
			CloseHandle(_file);
			_file = INVALID_HANDLE_VALUE;
#else
			fclose(_file);
			_file = NULL;
#endif
		}
	}

	bool is_open() override
	{
#if CROWN_PLATFORM_WINDOWS
		return _file != INVALID_HANDLE_VALUE;
#else
		return _file != NULL;
#endif
	}

	u32 size() override
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_WINDOWS
		return GetFileSize(_file, NULL);
#else
		Stat st;
		os::stat(st, fileno(_file));
		return (u32)st.size;
#endif
	}

	u32 position() override
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_WINDOWS
		DWORD pos = SetFilePointer(_file, 0, NULL, FILE_CURRENT);
		CE_ASSERT(pos != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
		return (u32)pos;
#else
		long pos = ftell(_file);
		CE_ASSERT(pos != -1, "ftell: errno = %d", errno);
		return (u32)pos;
#endif
	}

	bool end_of_file() override
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_WINDOWS
		return _eof;
#else
		return feof(_file) != 0;
#endif
	}

	void seek(u32 position) override
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, position, NULL, FILE_BEGIN);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#else
		int err = fseek(_file, (long)position, SEEK_SET);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#endif
		CE_UNUSED(err);
	}

	void seek_to_end() override
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, 0, NULL, FILE_END);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#else
		int err = fseek(_file, 0, SEEK_END);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#endif
		CE_UNUSED(err);
	}

	void skip(u32 bytes) override
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_WINDOWS
		DWORD err = SetFilePointer(_file, bytes, NULL, FILE_CURRENT);
		CE_ASSERT(err != INVALID_SET_FILE_POINTER
			, "SetFilePointer: GetLastError = %d"
			, GetLastError()
			);
#else
		int err = fseek(_file, bytes, SEEK_CUR);
		CE_ASSERT(err == 0, "fseek: errno = %d", errno);
#endif
		CE_UNUSED(err);
	}

	u32 read(void *data, u32 size) override
	{
		CE_ASSERT(is_open(), "File is not open");
		CE_ASSERT(data != NULL, "Data must be != NULL");
#if CROWN_PLATFORM_WINDOWS
		DWORD bytes_read;
		BOOL err = ReadFile(_file, data, size, &bytes_read, NULL);
		CE_ASSERT(err == TRUE, "ReadFile: GetLastError = %d", GetLastError());
		_eof = err && bytes_read == 0;
		return bytes_read;
#else
		size_t bytes_read = fread(data, 1, size, _file);
		CE_ASSERT(ferror(_file) == 0, "fread error");
		return (u32)bytes_read;
#endif
	}

	u32 write(const void *data, u32 size) override
	{
		CE_ASSERT(is_open(), "File is not open");
		CE_ASSERT(data != NULL, "Data must be != NULL");
#if CROWN_PLATFORM_WINDOWS
		DWORD bytes_written;
		WriteFile(_file, data, size, &bytes_written, NULL);
		CE_ASSERT(size == bytes_written
			, "WriteFile: GetLastError = %d"
			, GetLastError()
			);
		return bytes_written;
#else
		size_t bytes_written = fwrite(data, 1, size, _file);
		CE_ASSERT(ferror(_file) == 0, "fwrite error");
		return (u32)bytes_written;
#endif
	}

	s32 sync() override
	{
		CE_ASSERT(is_open(), "File is not open");
#if CROWN_PLATFORM_WINDOWS
		if (FlushFileBuffers(_file) != 0)
			return -1;
#else
		if (fflush(_file) != 0)
			return -1;
		if (fsync(fileno(_file)) == -1)
			return -1;
#endif
		return 0;
	}
};

FilesystemDisk::FilesystemDisk(Allocator &a)
	: _allocator(&a)
	, _prefix(a)
{
}

void FilesystemDisk::set_prefix(const char *prefix)
{
	_prefix.set(prefix, strlen32(prefix));
}

File *FilesystemDisk::open(const char *path, FileOpenMode::Enum mode)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	FileDisk *file = CE_NEW(*_allocator, FileDisk)();
	file->open(abs_path.c_str(), mode);
	return file;
}

File *FilesystemDisk::open_temporary(DynamicString &absolute_path)
{
	TempAllocator256 ta;
	DynamicString tmp_basename(ta);
	tmp_basename.from_guid(guid::new_guid());
	tmp_basename += ".tmp";
	this->absolute_path(absolute_path, tmp_basename.c_str());

	FileDisk *file = CE_NEW(*_allocator, FileDisk)();
	file->open(absolute_path.c_str(), FileOpenMode::WRITE);
	return file;
}

void FilesystemDisk::close(File &file)
{
	CE_DELETE(*_allocator, &file);
}

Stat FilesystemDisk::stat(const char *path)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	Stat st;
	os::stat(st, abs_path.c_str());
	return st;
}

bool FilesystemDisk::exists(const char *path)
{
	return stat(path).file_type != Stat::NO_ENTRY;
}

bool FilesystemDisk::is_directory(const char *path)
{
	return stat(path).file_type == Stat::DIRECTORY;
}

bool FilesystemDisk::is_file(const char *path)
{
	return stat(path).file_type == Stat::REGULAR;
}

u64 FilesystemDisk::last_modified_time(const char *path)
{
	return stat(path).mtime;
}

CreateResult FilesystemDisk::create_directory(const char *path)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	return os::create_directory(abs_path.c_str());
}

DeleteResult FilesystemDisk::delete_directory(const char *path)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	return os::delete_directory(abs_path.c_str());
}

DeleteResult FilesystemDisk::delete_file(const char *path)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	return os::delete_file(abs_path.c_str());
}

RenameResult FilesystemDisk::rename(const char *old_path, const char *new_path)
{
	CE_ENSURE(old_path != NULL);
	CE_ENSURE(new_path != NULL);

	DynamicString old_abs_path(default_allocator());
	DynamicString new_abs_path(default_allocator());
	absolute_path(old_abs_path, old_path);
	absolute_path(new_abs_path, new_path);

	return os::rename(old_abs_path.c_str(), new_abs_path.c_str());
}

void FilesystemDisk::list_files(const char *path, Vector<DynamicString> &files)
{
	CE_ENSURE(NULL != path);

	TempAllocator256 ta;
	DynamicString abs_path(ta);
	absolute_path(abs_path, path);

	os::list_files(abs_path.c_str(), files);
}

void FilesystemDisk::absolute_path(DynamicString &os_path, const char *path)
{
	if (path::is_absolute(path)) {
		os_path = path;
		return;
	}

	TempAllocator1024 ta;
	DynamicString str(ta);
	path::join(str, _prefix.c_str(), path);
	path::reduce(os_path, str.c_str());
}

} // namespace crown
