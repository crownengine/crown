/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "macros.h"
#include "platform.h"
#include "string_types.h"
#include "types.h"
#include <stdio.h>  // fputs

#if CROWN_PLATFORM_POSIX
	#include <dlfcn.h>    // dlopen, dlclose, dlsym
	#include <errno.h>
	#include <string.h>   // memset
	#include <sys/stat.h> // lstat, mknod, mkdir
	#include <sys/wait.h> // wait
	#include <time.h>     // clock_gettime
	#include <unistd.h>   // access, unlink, rmdir, getcwd, fork, execv
 	#include <stdlib.h>   // exit
#elif CROWN_PLATFORM_WINDOWS
	#include <io.h>
	#include <win_headers.h>
#endif
#if CROWN_PLATFORM_ANDROID
	#include <android/log.h>
#endif

namespace crown
{
namespace os
{
	inline void log(const char* msg)
	{
#if CROWN_PLATFORM_ANDROID
		__android_log_write(ANDROID_LOG_DEBUG, "crown", msg);
#else
		fputs(msg, stdout);
		fflush(stdout);
#endif
	}

	/// Returns whether the @a path exists.
	inline bool exists(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		return access(path, F_OK) != -1;
#elif CROWN_PLATFORM_WINDOWS
		return _access(path, 0) != -1;
#endif
	}

	/// Returns whether @a path is a directory.
	inline bool is_directory(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(info));
		int err = lstat(path, &info);
		CE_ASSERT(err == 0, "lstat: errno = %d", errno);
		CE_UNUSED(err);
		return ((S_ISDIR(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
#elif CROWN_PLATFORM_WINDOWS
		DWORD fattr = GetFileAttributes(path);
		return (fattr != INVALID_FILE_ATTRIBUTES && (fattr & FILE_ATTRIBUTE_DIRECTORY) != 0);
#endif
	}

	/// Returns whether @a path is a regular file.
	inline bool is_file(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(info));
		int err = lstat(path, &info);
		CE_ASSERT(err == 0, "lstat: errno = %d", errno);
		CE_UNUSED(err);
		return ((S_ISREG(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
#elif CROWN_PLATFORM_WINDOWS
		DWORD fattr = GetFileAttributes(path);
		return (fattr != INVALID_FILE_ATTRIBUTES && (fattr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#endif
	}

	/// Returns the last modification time of @a path.
	inline u64 mtime(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(info));
		int err = lstat(path, &info);
		CE_ASSERT(err == 0, "lstat: errno = %d", errno);
		CE_UNUSED(err);
		return info.st_mtime;
#elif CROWN_PLATFORM_WINDOWS
		HANDLE hfile = CreateFile(path
			, GENERIC_READ
			, FILE_SHARE_READ
			, NULL
			, OPEN_EXISTING
			, 0
			, NULL
			);
		CE_ASSERT(hfile != INVALID_HANDLE_VALUE, "CreateFile: GetLastError = %d", GetLastError());
		FILETIME ftwrite;
		BOOL err = GetFileTime(hfile, NULL, NULL, &ftwrite);
		CE_ASSERT(err != 0, "GetFileTime: GetLastError = %d", GetLastError());
		CE_UNUSED(err);
		CloseHandle(hfile);
		return (u64)((u64(ftwrite.dwHighDateTime) << 32) | ftwrite.dwLowDateTime);
#endif
	}

	/// Creates a regular file named @a path.
	inline void create_file(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		// Permission mask: rw-r--r--
		int err = ::mknod(path, S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0);
		CE_ASSERT(err == 0, "mknod: errno = %d", errno);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		HANDLE hfile = CreateFile(path
			, GENERIC_READ | GENERIC_WRITE
			, 0
			, NULL
			, CREATE_ALWAYS
			, FILE_ATTRIBUTE_NORMAL
			, NULL
			);
		CE_ASSERT(hfile != INVALID_HANDLE_VALUE, "CreateFile: GetLastError = %d", GetLastError());
		CloseHandle(hfile);
#endif
	}

	/// Deletes the file at @a path.
	inline void delete_file(const char* path)
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

	/// Creates a directory named @a path.
	inline void create_directory(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		// rwxr-xr-x
		int err = ::mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		CE_ASSERT(err == 0, "mkdir: errno = %d", errno);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		BOOL err = CreateDirectory(path, NULL);
		CE_ASSERT(err != 0, "CreateDirectory: GetLastError = %d", GetLastError());
		CE_UNUSED(err);
#endif
	}

	/// Deletes the directory at @a path.
	inline void delete_directory(const char* path)
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

	/// Returns the list of @a files at the given @a path.
	void list_files(const char* path, Vector<DynamicString>& files);

	/// Returns the current working directory.
	inline const char* getcwd(char* buf, u32 size)
	{
#if CROWN_PLATFORM_POSIX
		return ::getcwd(buf, size);
#elif CROWN_PLATFORM_WINDOWS
		GetCurrentDirectory(size, buf);
		return buf;
#endif
	}

	/// Returns the value of the environment variable @a name.
	inline const char* getenv(const char* name)
	{
#if CROWN_PLATFORM_POSIX
		return ::getenv(name);
#elif CROWN_PLATFORM_WINDOWS
		// GetEnvironmentVariable(name, buf, size);
#endif
	}

	inline s64 clocktime()
	{
#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_ANDROID
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * s64(1000000000) + now.tv_nsec;
#elif CROWN_PLATFORM_OSX
		struct timeval now;
		gettimeofday(&now, NULL);
		return now.tv_sec * s64(1000000) + now.tv_usec;
#elif CROWN_PLATFORM_WINDOWS
		LARGE_INTEGER ttime;
		QueryPerformanceCounter(&ttime);
		return (s64)ttime.QuadPart;
#endif
	}

	inline s64 clockfrequency()
	{
#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_ANDROID
		return s64(1000000000);
#elif CROWN_PLATFORM_OSX
		return s64(1000000);
#elif CROWN_PLATFORM_WINDOWS
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (s64)freq.QuadPart;
#endif
	}

	inline void* open_library(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		return ::dlopen(path, RTLD_LAZY);
#elif CROWN_PLATFORM_WINDOWS
		return (void*)LoadLibraryA(path);
#endif
	}

	inline void close_library(void* library)
	{
#if CROWN_PLATFORM_POSIX
		dlclose(library);
#elif CROWN_PLATFORM_WINDOWS
		FreeLibrary((HMODULE)library);
#endif
	}

	inline void* lookup_symbol(void* library, const char* name)
	{
#if CROWN_PLATFORM_POSIX
		return ::dlsym(library, name);
#elif CROWN_PLATFORM_WINDOWS
		return (void*)GetProcAddress((HMODULE)library, name);
#endif
	}

	/// Executes the process @a path with the given @a args and returns its exit code.
	/// It fills @a output with stdout and stderr.
	int execute_process(const char* path, const char* args, StringStream& output);
} // namespace os
} // namespace crown
