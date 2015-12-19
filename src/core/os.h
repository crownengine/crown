/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "platform.h"
#include "types.h"
#include "vector.h"
#include "dynamic_string.h"
#include "string_utils.h"
#include "error.h"
#include "temp_allocator.h"
#include "string_stream.h"

#if CROWN_PLATFORM_POSIX
	#include <dirent.h> // opendir, readdir
	#include <dlfcn.h> // dlopen, dlclose, dlsym
	#include <sys/stat.h> // lstat, mknod, mkdir
	#include <sys/wait.h> // wait
	#include <errno.h>
	#include <time.h> // clock_gettime
	#include <unistd.h> // access, unlink, rmdir, getcwd, fork, execv
 	#include <stdlib.h> // exit
#elif CROWN_PLATFORM_WINDOWS
	#include <win_headers.h>
	#include <io.h>
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
		puts(msg);
		fflush(stdout);
#endif
	}

	inline bool exists(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		return access(path, F_OK) != -1;
#elif CROWN_PLATFORM_WINDOWS
		return _access(path, 0) != -1;
#endif
	}

	/// Returns whether the path is a directory.
	inline bool is_directory(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(struct stat));
		int err = lstat(path, &info);
		CE_ASSERT(err == 0, "lstat: errno = %d", errno);
		CE_UNUSED(err);
		return ((S_ISDIR(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
#elif CROWN_PLATFORM_WINDOWS
		DWORD fattr = GetFileAttributes(path);
		return (fattr != INVALID_FILE_ATTRIBUTES && (fattr & FILE_ATTRIBUTE_DIRECTORY) != 0);
#endif
	}

	/// Returns whether the path is a regular file.
	inline bool is_file(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(struct stat));
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
	inline uint64_t mtime(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		struct stat info;
		memset(&info, 0, sizeof(struct stat));
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
		return (uint64_t)((ftwrite.dwHighDateTime << 32) | ftwrite.dwLowDateTime);
#endif
	}

	/// Creates a regular file.
	inline void create_file(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		// Permission mask: rw-r--r--
		int err = ::mknod(path, S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0);
		CE_ASSERT(err == 0, "mknod: errno = %d", errno);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		HANDLE hfile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		CE_ASSERT(hfile != INVALID_HANDLE_VALUE, "CreateFile: GetLastError = %d", GetLastError());
		CloseHandle(hfile);
#endif
	}

	/// Deletes a regular file.
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

	/// Creates a directory.
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

	/// Deletes a directory.
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

	/// Returns the list of @a files in the given @a dir directory. Optionally walks into
	/// subdirectories whether @a recursive is true.
	/// @note
	/// Does not follow symbolic links.
	inline void list_files(const char* path, Vector<DynamicString>& files)
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
			DynamicString fname(dname, ta);
			vector::push_back(files, fname);
		}

		closedir(dir);
#elif CROWN_PLATFORM_WINDOWS
		TempAllocator1024 ta;
		DynamicString cur_path(path, ta);
		cur_path += "\\*";

		WIN32_FIND_DATA ffd;
		HANDLE file = FindFirstFile(cur_path.c_str(), &ffd);
		if (file == INVALID_HANDLE_VALUE)
			return;

		do
		{
			const char* fname = ffd.cFileName;

			if (!strcmp(fname, ".") || !strcmp(fname, ".."))
				continue;

			TempAllocator512 ta;
			DynamicString filename(fname, ta);
			vector::push_back(files, fname);
		}
		while (FindNextFile(file, &ffd) != 0);

		FindClose(file);
#endif
	}

	inline const char* getcwd(char* buf, uint32_t size)
	{
#if CROWN_PLATFORM_POSIX
		return ::getcwd(buf, size);
#elif CROWN_PLATFORM_WINDOWS
		GetCurrentDirectory(size, buf);
		return buf;
#endif
	}

	inline const char* getenv(const char* name)
	{
#if CROWN_PLATFORM_POSIX
		return ::getenv(name);
#elif CROWN_PLATFORM_WINDOWS
		// GetEnvironmentVariable(name, buf, size);
#endif
	}

	inline int64_t clocktime()
	{
#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_ANDROID
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * int64_t(1000000000) + now.tv_nsec;
#elif CROWN_PLATFORM_OSX
		struct timeval now;
		gettimeofday(&now, NULL);
		return now.tv_sec * int64_t(1000000) + now.tv_usec;
#elif CROWN_PLATFORM_WINDOWS
		LARGE_INTEGER ttime;
		QueryPerformanceCounter(&ttime);
		return (int64_t)ttime.QuadPart;
#endif
	}

	inline int64_t clockfrequency()
	{
#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_ANDROID
		return int64_t(1000000000);
#elif CROWN_PLATFORM_OSX
		return int64_t(1000000);
#elif CROWN_PLATFORM_WINDOWS
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (int64_t)freq.QuadPart;
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

	/// Executes a process.
	inline int execute_process(const char* path, const char* args, StringStream& output)
	{
#if CROWN_PLATFORM_POSIX
		using namespace string_stream;

		TempAllocator512 ta;
		DynamicString cmd(path, ta);
		cmd += " 2>&1 ";
		cmd += args;
		FILE* file = popen(cmd.c_str(), "r");

		char buf[1024];
		while (fgets(buf, sizeof(buf), file) != NULL)
			output << buf;

		return pclose(file);
#elif CROWN_PLATFORM_WINDOWS
		STARTUPINFO info;
		memset(&info, 0, sizeof(info));
		info.cb = sizeof(info);

		PROCESS_INFORMATION process;
		memset(&process, 0, sizeof(process));

		int err = CreateProcess(path, args, NULL, NULL, TRUE, 0, NULL, NULL, &info, &process);
		CE_ASSERT(err != 0, "CreateProcess: GetLastError = %d", GetLastError());
		CE_UNUSED(err);

		DWORD exitcode = 1;
		::WaitForSingleObject(process.hProcess, INFINITE);
  		GetExitCodeProcess(process.hProcess, &exitcode);
		CloseHandle(process.hProcess);
		CloseHandle(process.hThread);
		return (int)exitcode;
#endif
	}
} // namespace os
} // namespace crown
