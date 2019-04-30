/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/vector.h"
#include "core/error/error.h"
#include "core/memory/temp_allocator.h"
#include "core/os.h"
#include "core/platform.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_stream.h"
#include <string.h>   // strcmp
#include <sys/stat.h> // stat, mkdir

#if CROWN_PLATFORM_POSIX
	#include <dirent.h>   // opendir, readdir
	#include <dlfcn.h>    // dlopen, dlclose, dlsym
	#include <errno.h>
	#include <stdio.h>    // fputs
	#include <stdlib.h>   // getenv
	#include <string.h>   // memset
	#include <sys/wait.h> // wait
	#include <time.h>     // clock_gettime
	#include <unistd.h>   // unlink, rmdir, getcwd, access
#elif CROWN_PLATFORM_WINDOWS
	#include <io.h>       // _access
	#include <stdio.h>
	#include <windows.h>
#endif
#if CROWN_PLATFORM_ANDROID
	#include <android/log.h>
#endif

namespace crown
{
namespace os
{
	void sleep(u32 ms)
	{
#if CROWN_PLATFORM_POSIX
		usleep(ms * 1000);
#elif CROWN_PLATFORM_WINDOWS
		Sleep(ms);
#endif
	}

	void* library_open(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		return ::dlopen(path, RTLD_LAZY);
#elif CROWN_PLATFORM_WINDOWS
		return (void*)LoadLibraryA(path);
#endif
	}

	void library_close(void* library)
	{
#if CROWN_PLATFORM_POSIX
		dlclose(library);
#elif CROWN_PLATFORM_WINDOWS
		FreeLibrary((HMODULE)library);
#endif
	}

	void* library_symbol(void* library, const char* name)
	{
#if CROWN_PLATFORM_POSIX
		return ::dlsym(library, name);
#elif CROWN_PLATFORM_WINDOWS
		return (void*)GetProcAddress((HMODULE)library, name);
#endif
	}

	void log(const char* msg)
	{
#if CROWN_PLATFORM_ANDROID
		__android_log_write(ANDROID_LOG_DEBUG, "crown", msg);
#elif CROWN_PLATFORM_WINDOWS
		OutputDebugStringA(msg);
#else
		fputs(msg, stdout);
		fflush(stdout);
#endif
	}

	void stat(Stat& info, const char* path)
	{
		info.file_type = Stat::NO_ENTRY;
		info.size = 0;
		info.mtime = 0;

#if CROWN_PLATFORM_POSIX
		struct stat buf;
		memset(&buf, 0, sizeof(buf));
		int err = ::stat(path, &buf);
		if (err != 0)
			return;

		if (S_ISREG(buf.st_mode) == 1)
			info.file_type = Stat::REGULAR;
		else if (S_ISDIR(buf.st_mode) == 1)
			info.file_type = Stat::DIRECTORY;
#elif CROWN_PLATFORM_WINDOWS
		struct _stat64 buf;
		int err = ::_stat64(path, &buf);
		if (err != 0)
			return;

		if ((buf.st_mode & _S_IFREG) != 0)
			info.file_type = Stat::REGULAR;
		else if ((buf.st_mode & _S_IFDIR) != 0)
			info.file_type = Stat::DIRECTORY;
#endif

		info.size = buf.st_size;
		info.mtime = buf.st_mtime;
	}

	void delete_file(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		int err = ::unlink(path);
		CE_ASSERT(err == 0, "unlink: errno = %d", errno);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		BOOL err = DeleteFile(path);
		CE_ASSERT(err != 0, "DeleteFile: GetLastError = %d", GetLastError());
		CE_UNUSED(err);
#endif
	}

	void create_directory(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		int err = ::mkdir(path, 0755);
		CE_ASSERT(err == 0, "mkdir: errno = %d", errno);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		BOOL err = CreateDirectory(path, NULL);
		CE_ASSERT(err != 0, "CreateDirectory: GetLastError = %d", GetLastError());
		CE_UNUSED(err);
#endif
	}

	void delete_directory(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		int err = ::rmdir(path);
		CE_ASSERT(err == 0, "rmdir: errno = %d", errno);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		BOOL err = RemoveDirectory(path);
		CE_ASSERT(err != 0, "RemoveDirectory: GetLastError = %d", GetLastError());
		CE_UNUSED(err);
#endif
	}

	const char* getcwd(char* buf, u32 size)
	{
#if CROWN_PLATFORM_POSIX
		return ::getcwd(buf, size);
#elif CROWN_PLATFORM_WINDOWS
		GetCurrentDirectory(size, buf);
		return buf;
#endif
	}

	const char* getenv(const char* name)
	{
#if CROWN_PLATFORM_POSIX
		return ::getenv(name);
#elif CROWN_PLATFORM_WINDOWS
		// GetEnvironmentVariable(name, buf, size);
		return NULL;
#endif
	}

	void list_files(const char* path, Vector<DynamicString>& files)
	{
#if CROWN_PLATFORM_POSIX
		DIR *dir;
		struct dirent *entry;

		if (!(dir = opendir(path)))
			return;

		while ((entry = readdir(dir)))
		{
			const char* dname = entry->d_name;

			if (!strcmp(dname, ".") || !strcmp(dname, ".."))
				continue;

			TempAllocator512 ta;
			DynamicString fname(ta);
			fname.set(dname, strlen32(dname));
			vector::push_back(files, fname);
		}

		closedir(dir);
#elif CROWN_PLATFORM_WINDOWS
		TempAllocator1024 ta;
		DynamicString cur_path(ta);
		cur_path += path;
		cur_path += "\\*";

		WIN32_FIND_DATA ffd;
		HANDLE file = FindFirstFile(cur_path.c_str(), &ffd);
		if (file == INVALID_HANDLE_VALUE)
			return;

		do
		{
			const char* dname = ffd.cFileName;

			if (!strcmp(dname, ".") || !strcmp(dname, ".."))
				continue;

			TempAllocator512 ta;
			DynamicString fname(ta);
			fname.set(dname, strlen32(dname));
			vector::push_back(files, fname);
		}
		while (FindNextFile(file, &ffd) != 0);

		FindClose(file);
#endif
	}

	///
	s32 access(const char* path, u32 flags)
	{
#if CROWN_PLATFORM_POSIX
		return ::access(path, flags);
#elif CROWN_PLATFORM_WINDOWS
		return ::_access(path, flags == AccessFlags::EXECUTE ? AccessFlags::EXISTS : flags);
#endif
	}

} // namespace os

} // namespace crown
