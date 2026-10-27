/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if !CROWN_PLATFORM_EMSCRIPTEN
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/http_file.h"
#include "core/memory/globals.h"
#include "core/os.h"
#include "core/strings/dynamic_string.inl"

#if CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#elif CROWN_PLATFORM_ANDROID
	#include "device/types.h"
#endif

namespace crown
{
#if CROWN_PLATFORM_ANDROID
extern PlatformData g_platform_data;
#endif

namespace http_file
{
	bool open(HttpDownload &download, Allocator &a, const char *path)
	{
		if (download.file != NULL)
			return true;
		FilesystemDisk fs(a);
		File *file = fs.open(path, FileOpenMode::WRITE);
		if (!file->is_open()) {
			fs.close(*file);
			return false;
		}
		download.file = file;
		download.allocator = &a;
		download.path = path;
		return true;
	}

	bool write(HttpDownload &download, const void *data, u32 size)
	{
		if (download.size + size > UINT32_MAX
			|| download.file->write(data, size) != size)
			return false;
		download.size += size;
		return true;
	}

	bool finish(HttpDownload &download)
	{
		if (download.file == NULL)
			return false;
		FilesystemDisk fs(*download.allocator);
		fs.close(*download.file);
		download.file = NULL;
		return size(download.path) == (s64)download.size;
	}

	void close(HttpDownload &download)
	{
		if (download.allocator == NULL)
			return;
		FilesystemDisk fs(*download.allocator);
		if (download.file != NULL)
			fs.close(*download.file);
		if (!download.complete)
			fs.delete_file(download.path);
		download.file = NULL;
		download.path = NULL;
		download.allocator = NULL;
	}

	bool temporary_path(DynamicString &path)
	{
#if CROWN_PLATFORM_WINDOWS
		char directory[MAX_PATH];
		const DWORD size = GetTempPathA(sizeof(directory), directory);
		if (size == 0 || size >= sizeof(directory))
			return false;
#elif CROWN_PLATFORM_ANDROID
		const char *directory = (const char *)g_platform_data._android_internal_data_path;
		if (directory == NULL)
			return false;
#else
		const char *directory = os::getenv("TMPDIR");
		if (directory == NULL)
			directory = "/tmp";
#endif
		FilesystemDisk fs(default_allocator());
		fs.set_prefix(directory);
		path = "";
		File *file = fs.open_temporary(path);
		const bool success = file->is_open();
		fs.close(*file);
		return success;
	}

	s64 size(const char *path)
	{
		FilesystemDisk fs(default_allocator());
		const Stat st = fs.stat(path);
		return st.file_type == Stat::REGULAR && st.size <= UINT32_MAX ? (s64)st.size : -1;
	}

	u32 read(void *data, HttpReadFile &reader, const char *path, u32 offset, u32 size)
	{
		if (reader.file == NULL) {
			FilesystemDisk fs(default_allocator());
			reader.file = fs.open(path, FileOpenMode::READ);
			if (!reader.file->is_open()) {
				fs.close(*reader.file);
				reader.file = NULL;
				return 0;
			}
		}
		if (reader.position != offset)
			reader.file->seek(offset);
		const u32 count = reader.file->read(data, size);
		reader.position = offset + count;
		return count;
	}

	void remove(const char *path, HttpReadFile &reader)
	{
		FilesystemDisk fs(default_allocator());
		if (reader.file != NULL) {
			fs.close(*reader.file);
			reader.file = NULL;
			reader.position = 0;
		}
		fs.delete_file(path);
	}

} // namespace http_file

} // namespace crown
#endif // if !CROWN_PLATFORM_EMSCRIPTEN
