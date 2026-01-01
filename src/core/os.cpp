/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/vector.inl"
#include "core/error/error.h"
#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/platform.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.h"
#include <string.h>   // strcmp
#include <sys/stat.h> // stat, mkdir

#if CROWN_PLATFORM_WINDOWS
	#include <io.h>       // _access
	#include <stdio.h>
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#else
	#include <dirent.h>   // opendir, readdir
	#include <dlfcn.h>    // dlopen, dlclose, dlsym
	#include <errno.h>
	#include <stdio.h>    // fputs, rename
	#include <stdlib.h>   // getenv
	#include <string.h>   // memset
	#include <sys/wait.h> // wait
	#include <time.h>     // clock_gettime
	#include <unistd.h>   // unlink, rmdir, getcwd, access, chdir
#endif // if CROWN_PLATFORM_WINDOWS
#if CROWN_PLATFORM_ANDROID
	#include <android/log.h>
#endif

namespace crown
{
namespace os
{
	void sleep(u32 ms)
	{
#if CROWN_PLATFORM_WINDOWS
		Sleep(ms);
#else
		usleep(ms * 1000);
#endif
	}

	void *library_open(const char *path)
	{
#if CROWN_PLATFORM_WINDOWS
		return (void *)LoadLibraryA(path);
#else
		return ::dlopen(path, RTLD_LAZY);
#endif
	}

	void library_close(void *library)
	{
#if CROWN_PLATFORM_WINDOWS
		BOOL err = FreeLibrary((HMODULE)library);
		CE_ASSERT(err != 0, "FreeLibrary: error: %s", GetLastError());
#else
		int err = dlclose(library);
		CE_ASSERT(err == 0, "dlclose: error: %s", dlerror());
#endif
		CE_UNUSED(err);
	}

	void *library_symbol(void *library, const char *name)
	{
#if CROWN_PLATFORM_WINDOWS
		return (void *)GetProcAddress((HMODULE)library, name);
#else
		return ::dlsym(library, name);
#endif
	}

	void log(const char *msg)
	{
#if CROWN_PLATFORM_ANDROID
		__android_log_write(ANDROID_LOG_DEBUG, "crown", msg);
#else
		fputs(msg, stdout);
		fflush(stdout);
#endif
	}

#if CROWN_PLATFORM_POSIX
	void stat(Stat &st, int fd)
	{
		st.file_type = Stat::NO_ENTRY;
		st.size = 0;
		st.mtime = 0;

		struct stat buf;
		memset(&buf, 0, sizeof(buf));
		int err = ::fstat(fd, &buf);
		if (err != 0)
			return;

		if (S_ISREG(buf.st_mode) == 1)
			st.file_type = Stat::REGULAR;
		else if (S_ISDIR(buf.st_mode) == 1)
			st.file_type = Stat::DIRECTORY;

		st.size  = buf.st_size;
		st.mtime = buf.st_mtim.tv_sec * s64(1000000000) + buf.st_mtim.tv_nsec;
	}
#endif // if CROWN_PLATFORM_POSIX

	void stat(Stat &st, const char *path)
	{
		st.file_type = Stat::NO_ENTRY;
		st.size  = 0;
		st.mtime = 0;

#if CROWN_PLATFORM_WINDOWS
		WIN32_FIND_DATAA wfd;
		HANDLE fh = FindFirstFileA(path, &wfd);
		if (fh == INVALID_HANDLE_VALUE)
			return;
		FindClose(fh);

		if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			st.file_type = Stat::DIRECTORY;
		else // Assume regular file.
			st.file_type = Stat::REGULAR;

		ULARGE_INTEGER fs = {};
		fs.LowPart  = wfd.nFileSizeLow;
		fs.HighPart = wfd.nFileSizeHigh;
		st.size = fs.QuadPart;

		ULARGE_INTEGER lwt = {};
		lwt.LowPart  = wfd.ftLastWriteTime.dwLowDateTime;
		lwt.HighPart = wfd.ftLastWriteTime.dwHighDateTime;
		// Convert mtime to ns and subtract Unix epoch to Windows epoch
		// difference in ns (i.e. 1970-01-01, 12AM - 1601-01-01, 12AM).
		// See: https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime
		st.mtime = lwt.QuadPart * u64(100) - u64(11644473600) * u64(1000000000);
#else
		struct stat buf;
		memset(&buf, 0, sizeof(buf));
		int err = ::stat(path, &buf);
		if (err != 0)
			return;

		if (S_ISREG(buf.st_mode) == 1)
			st.file_type = Stat::REGULAR;
		else if (S_ISDIR(buf.st_mode) == 1)
			st.file_type = Stat::DIRECTORY;

		st.size  = buf.st_size;
		st.mtime = buf.st_mtim.tv_sec * s64(1000000000) + buf.st_mtim.tv_nsec;
#endif // if CROWN_PLATFORM_WINDOWS
	}

	DeleteResult delete_file(const char *path)
	{
		DeleteResult dr;
#if CROWN_PLATFORM_WINDOWS
		if (DeleteFile(path) != 0)
			dr.error = DeleteResult::SUCCESS;
		else if (GetLastError() == ERROR_FILE_NOT_FOUND)
			dr.error = DeleteResult::NO_ENTRY;
		// else if (GetLastError() == ERROR_ACCESS_DENIED)
		// 	dr.error = DeleteResult::NOT_FILE;
		else
			dr.error = DeleteResult::UNKNOWN;
#else
		if (::unlink(path) == 0)
			dr.error = DeleteResult::SUCCESS;
		else if (errno == ENOENT)
			dr.error = DeleteResult::NO_ENTRY;
		else
			dr.error = DeleteResult::UNKNOWN;
#endif
		return dr;
	}

	CreateResult create_directory(const char *path)
	{
		CreateResult cr;
#if CROWN_PLATFORM_WINDOWS
		if (CreateDirectory(path, NULL) != 0)
			cr.error = CreateResult::SUCCESS;
		else if (GetLastError() == ERROR_ALREADY_EXISTS)
			cr.error = CreateResult::ALREADY_EXISTS;
		else
			cr.error = CreateResult::UNKNOWN;
#else
		if (::mkdir(path, 0755) == 0)
			cr.error = CreateResult::SUCCESS;
		else if (errno == EEXIST)
			cr.error = CreateResult::ALREADY_EXISTS;
		else
			cr.error = CreateResult::UNKNOWN;
#endif
		return cr;
	}

	DeleteResult delete_directory(const char *path)
	{
		DeleteResult dr;
#if CROWN_PLATFORM_WINDOWS
		if (RemoveDirectory(path) != 0)
			dr.error = DeleteResult::SUCCESS;
		else if (GetLastError() == ERROR_FILE_NOT_FOUND)
			dr.error = DeleteResult::NO_ENTRY;
		// else if (GetLastError() == ERROR_DIRECTORY
		// 	dr.error = DeleteResult::NOT_DIRECTORY;
		else
			dr.error = DeleteResult::UNKNOWN;
#else
		if (::rmdir(path) == 0)
			dr.error = DeleteResult::SUCCESS;
		else if (errno == ENOENT)
			dr.error = DeleteResult::NO_ENTRY;
		else
			dr.error = DeleteResult::UNKNOWN;
#endif
		return dr;
	}

	const char *getcwd(char *buf, u32 size)
	{
#if CROWN_PLATFORM_WINDOWS
		GetCurrentDirectory(size, buf);
		return buf;
#else
		return ::getcwd(buf, size);
#endif
	}

	void setcwd(const char *cwd)
	{
#if CROWN_PLATFORM_WINDOWS
		BOOL ret = SetCurrentDirectory(cwd);
		CE_UNUSED(ret);
#else
		int ret = chdir(cwd);
		CE_UNUSED(ret);
#endif
	}

	const char *getenv(const char *name)
	{
#if CROWN_PLATFORM_WINDOWS
		// GetEnvironmentVariable(name, buf, size);
		return NULL;
#else
		return ::getenv(name);
#endif
	}

	s32 setenv(const char *name, const char *value)
	{
#if CROWN_PLATFORM_WINDOWS
		return SetEnvironmentVariable(name, value) != 0 ? 0 : -1;
#else
		return ::setenv(name, value, 1);
#endif
	}

	void list_files(const char *path, Vector<DynamicString> &files)
	{
#if CROWN_PLATFORM_WINDOWS
		TempAllocator256 ta_path;
		DynamicString cur_path(ta_path);
		cur_path += path;
		cur_path += "\\*";

		WIN32_FIND_DATA ffd;
		HANDLE file = FindFirstFile(cur_path.c_str(), &ffd);
		if (file != INVALID_HANDLE_VALUE) {
			do {
				const char *dname = ffd.cFileName;

				if (!strcmp(dname, ".") || !strcmp(dname, ".."))
					continue;

				TempAllocator256 ta;
				DynamicString fname(ta);
				fname.set(dname, strlen32(dname));
				vector::push_back(files, fname);
			} while (FindNextFile(file, &ffd) != 0);

			FindClose(file);
		}
#else
		struct dirent *entry;

		DIR *dir = opendir(path);
		if (dir != NULL) {
			while ((entry = readdir(dir))) {
				const char *dname = entry->d_name;

				if (!strcmp(dname, ".") || !strcmp(dname, ".."))
					continue;

				TempAllocator256 ta;
				DynamicString fname(ta);
				fname.set(dname, strlen32(dname));
				vector::push_back(files, fname);
			}

			closedir(dir);
		}
#endif // if CROWN_PLATFORM_WINDOWS
	}

	///
	s32 access(const char *path, u32 flags)
	{
#if CROWN_PLATFORM_WINDOWS
		return ::_access(path, flags == AccessFlags::EXECUTE ? AccessFlags::EXISTS : flags);
#else
		return ::access(path, flags);
#endif
	}

	RenameResult rename(const char *old_name, const char *new_name)
	{
		RenameResult rr;
#if CROWN_PLATFORM_WINDOWS
		if (MoveFileEx(old_name, new_name, MOVEFILE_REPLACE_EXISTING) != 0)
			rr.error = RenameResult::SUCCESS;
		else
			rr.error = RenameResult::UNKNOWN;
#else
		if (::rename(old_name, new_name) == 0)
			rr.error = RenameResult::SUCCESS;
		else
			rr.error = RenameResult::UNKNOWN;
#endif
		return rr;
	}

} // namespace os

} // namespace crown
