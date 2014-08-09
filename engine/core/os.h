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

#include "config.h"
#include "types.h"
#include "vector.h"
#include "dynamic_string.h"
#include "string_utils.h"
#include "temp_allocator.h"

#if CROWN_PLATFORM_POSIX
	#include "assert.h"
	#include <cstdarg>
	#include <cstdio>
	#include <cstdlib>
	#include <dirent.h>
	#include <dlfcn.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <errno.h>
	#include <time.h>
	#include <unistd.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <win_headers.h>
	#include <io.h>
#endif

#if CROWN_PLATFORM_ANDROID
	#include <android/log.h>
#endif

namespace crown
{

#if CROWN_PLATFORM_LINUX
	const size_t	MAX_PATH_LENGTH = 1024;
	const char		PATH_SEPARATOR = '/';
#elif CROWN_PLATFORM_WINDOWS
	const size_t	MAX_PATH_LENGTH = 1024;
	const char		PATH_SEPARATOR = '\\';
	#define snprintf _snprintf
#elif CROWN_PLATFORM_ANDROID
	const size_t	MAX_PATH_LENGTH = 1024;
	const char		PATH_SEPARATOR = '/';
#endif

namespace os
{

//-----------------------------------------------------------------------------
inline void log_debug(const char* string, va_list arg)
{
#if CROWN_PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_DEBUG, "crown", string, arg);
#else
	vprintf(string, arg);
	printf("\n");
#endif
}

//-----------------------------------------------------------------------------
inline void log_error(const char* string, va_list arg)
{
#if CROWN_PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_ERROR, "crown", string, arg);
#else
	vprintf(string, arg);
	printf("\n");
#endif
}

//-----------------------------------------------------------------------------
inline void log_warning(const char* string, va_list arg)
{
#if CROWN_PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_WARN, "crown", string, arg);
#else
	vprintf(string, arg);
	printf("\n");
#endif
}

//-----------------------------------------------------------------------------
inline void log_info(const char* string, va_list arg)
{
#if CROWN_PLATFORM_ANDROID
	__android_log_vprint(ANDROID_LOG_INFO, "crown", string, arg);
#else
	vprintf(string, arg);
	printf("\n");
#endif
}

//-----------------------------------------------------------------------------
inline bool is_root_path(const char* path)
{
#if CROWN_PLATFORM_POSIX
	return (path != NULL && string::strlen(path) == 1 && path[0] == PATH_SEPARATOR);
#elif CROWN_PLATFORM_WINDOWS
	return (path != NULL && string::strlen(path) == 3 && string::is_alpha(path[0]) &&
		path[1] == ':' && path[2] == PATH_SEPARATOR);
#endif
}

//-----------------------------------------------------------------------------
inline bool is_absolute_path(const char* path)
{
#if CROWN_PLATFORM_POSIX
	return (path != NULL && string::strlen(path) >= 1 && path[0] == PATH_SEPARATOR);
#elif CROWN_PLATFORM_WINDOWS
	return (path != NULL && string::strlen(path) >= 3 && string::is_alpha(path[0]) &&
		path[1] == ':' && path[2] == PATH_SEPARATOR);
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

/// Creates a directory. Returns true if success, false if not
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

/// Deletes a directory. Returns true if success, false if not
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
	{
		return;
	}

	while ((entry = readdir(dir)))
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}

		DynamicString filename(default_allocator());

		filename = entry->d_name;
		vector::push_back(files, filename);
	}

	closedir(dir);
#elif CROWN_PLATFORM_WINDOWS
	HANDLE file = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA ffd;

	char cur_path[MAX_PATH_LENGTH];

	string::strncpy(cur_path, path, string::strlen(path) + 1);
	string::strncat(cur_path, "\\*", 2);

	file = FindFirstFile(cur_path, &ffd);

	do
	{
		CE_ASSERT(file != INVALID_HANDLE_VALUE, "Unable to list files. errono %d", GetLastError());

		if ((string::strcmp(ffd.cFileName, ".") == 0) || (string::strcmp(ffd.cFileName, "..") == 0))
		{
			continue;
		}

		DynamicString filename(default_allocator());

		filename = ffd.cFileName;
		vector::push_back(files, filename);
	}
	while (FindNextFile(file, &ffd) != 0);

	FindClose(file);
#endif
}

/// Returns os-dependent path from os-indipendent @a path
inline const char* normalize_path(const char* path)
{
#if CROWN_PLATFORM_POSIX
	static char norm[MAX_PATH_LENGTH];
	char* cur = norm;

	while ((*path) != '\0')
	{
		if ((*path) == '\\')
		{
			(*cur) = PATH_SEPARATOR;
		}
		else
		{
			(*cur) = (*path);
		}

		path++;
		cur++;
	}

	return norm;
#elif CROWN_PLATFORM_WINDOWS
	static char norm[MAX_PATH_LENGTH];

	for (uint32_t i = 0; i < string::strlen(path)+1; i++)
	{
		if (path[i] == '/')
		{
			norm[i] = '\\';
		}
		else
		{
			norm[i] = path[i];
		}
	}

	return norm;
#endif
}

inline const char* getcwd(char* buf, size_t size)
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
#if CROWN_PLATFORM_POSIX
	timespec ttime;
	clock_gettime(CLOCK_MONOTONIC, &ttime);
	return ttime.tv_sec * int64_t(1000000000) + ttime.tv_nsec;
#elif CROWN_PLATFORM_WINDOWS
	LARGE_INTEGER ttime;
	QueryPerformanceCounter(&ttime);
	return (int64_t) ttime.QuadPart;
#endif
}

inline int64_t clockfrequency()
{
#if CROWN_PLATFORM_POSIX
	return int32_t(1000000000);
#elif CROWN_PLATFORM_WINDOWS
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (int64_t) freq.QuadPart;
#endif
}

inline void* open_library(const char* path)
{
#if CROWN_PLATFORM_POSIX
	return ::dlopen(path, RTLD_LAZY);
#elif CROWN_PLATFORM_WINDOWS
	return (void*) LoadLibraryA(path);
#endif
}

inline void close_library(void* library)
{
#if CROWN_PLATFORM_POSIX
	dlclose(library);
#elif CROWN_PLATFORM_WINDOWS
	FreeLibrary((HMODULE) library);
#endif
}

inline void* lookup_symbol(void* library, const char* name)
{
#if CROWN_PLATFORM_POSIX
	return ::dlsym(library, name);
#elif CROWN_PLATFORM_WINDOWS
	return (void*) GetProcAddress((HMODULE) library, name);
#endif
}

/// Executes a process.
/// @a args is an array of arguments where:
/// @a args[0] is the path to the program executable,
/// @a args[1, 2, ..., n-1] is a list of arguments to pass to the executable,
/// @a args[n] is NULL.
inline void execute_process(const char* args[])
{
#if CROWN_PLATFORM_POSIX
	pid_t pid = fork();
	CE_ASSERT(pid != -1, "fork: errno = %d", errno);
	if (pid)
	{
		int32_t dummy;
		wait(&dummy);
	}
	else
	{
		int err = execv(args[0], (char* const*)args);
		CE_ASSERT(err != -1, "execv: errno = %d", errno);
		CE_UNUSED(err);
		exit(EXIT_SUCCESS);
	}
#elif CROWN_PLATFORM_WINDOWS
	STARTUPINFO info;
	memset(&info, 0, sizeof(info));
	info.cb = sizeof(info);

	PROCESS_INFORMATION process;
	memset(&process, 0, sizeof(process));

	DynamicString cmds(default_allocator());

	for (uint32_t i = 0; args[i] != NULL; i++)
	{
		cmds += args[i];
		cmds += ' ';
	}

	// Necessary because CreateProcess second argument must be non-const
	char tmp[1024];
	string::strncpy(tmp, normalize_path(cmds.c_str()), string::strlen(cmds.c_str()));

	int err = CreateProcess(args[0], tmp, NULL, NULL, TRUE, 0, NULL, NULL, &info, &process);
	CE_ASSERT(err != 0, "CreateProcess: GetLastError = %d", GetLastError());
	CE_UNUSED(err);

	::WaitForSingleObject(process.hProcess, INFINITE);
	CloseHandle(process.hProcess);
	CloseHandle(process.hThread);
#endif
}

} // namespace os
} // namespace crown
