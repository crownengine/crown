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

#include "Assert.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "OS.h"
#include "StringUtils.h"

namespace crown
{
namespace os
{

static timespec base_time;

//-----------------------------------------------------------------------------
void printf(const char* string, ...)
{
	va_list args;

	va_start(args, string);
	::vprintf(string, args);
	va_end(args);
}

//-----------------------------------------------------------------------------
void vprintf(const char* string, va_list arg)
{
	::vprintf(string, arg);
}

//-----------------------------------------------------------------------------
void log_debug(const char* string, va_list arg)
{
	printf("D: ");
	vprintf(string, arg);
	printf("\n");
}

//-----------------------------------------------------------------------------
void log_error(const char* string, va_list arg)
{
	printf("E: ");
	vprintf(string, arg);
	printf("\n");
}

//-----------------------------------------------------------------------------
void log_warning(const char* string, va_list arg)
{
	printf("W: ");
	vprintf(string, arg);
	printf("\n");
}

//-----------------------------------------------------------------------------
void log_info(const char* string, va_list arg)
{
	printf("I: ");
	vprintf(string, arg);
	printf("\n");
}

//-----------------------------------------------------------------------------
bool is_root_path(const char* path)
{
	CE_ASSERT(path != NULL, "Path must be != NULL");

	if (string::strlen(path) == 1)
	{
		if (path[0] == PATH_SEPARATOR)
		{
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool is_absolute_path(const char* path)
{
	CE_ASSERT(path != NULL, "Path must be != NULL");

	if (string::strlen(path) > 0)
	{
		if (path[0] == PATH_SEPARATOR)
		{
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool exists(const char* path)
{
	struct stat dummy;
	return (stat(path, &dummy) == 0);
}

//-----------------------------------------------------------------------------
bool is_dir(const char* path)
{
	struct stat info;
	memset(&info, 0, sizeof(struct stat));
	lstat(path, &info);
	return ((S_ISDIR(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
}

//-----------------------------------------------------------------------------
bool is_reg(const char* path)
{
	struct stat info;
	memset(&info, 0, sizeof(struct stat));
	lstat(path, &info);
	return ((S_ISREG(info.st_mode) == 1) && (S_ISLNK(info.st_mode) == 0));
}

//-----------------------------------------------------------------------------
bool mknod(const char* path)
{
	// Permission mask: rw-r--r--
	return ::mknod(path, S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0) == 0;
}

//-----------------------------------------------------------------------------
bool unlink(const char* path)
{
	return (::unlink(path) == 0);
}

//-----------------------------------------------------------------------------
bool mkdir(const char* path)
{
	// rwxr-xr-x permission mask
	return (::mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0);
}

//-----------------------------------------------------------------------------
bool rmdir(const char* path)
{
	return (::rmdir(path) == 0);
}

//-----------------------------------------------------------------------------
const char* get_cwd()
{
	static char cwdBuf[MAX_PATH_LENGTH];
	if (getcwd(cwdBuf, MAX_PATH_LENGTH) == NULL)
	{
		return string::EMPTY;
	}

	return cwdBuf;
}

//-----------------------------------------------------------------------------
const char* get_home()
{
	char* envHome = NULL;
	envHome = getenv("HOME");

	if (envHome == NULL)
	{
		return string::EMPTY;
	}

	return envHome;
}

//-----------------------------------------------------------------------------
const char* get_env(const char* env)
{
	char* envDevel = NULL;
	envDevel = getenv(env);

	if (envDevel == NULL)
	{
		return string::EMPTY;
	}

	return envDevel;
}

//-----------------------------------------------------------------------------
void init_os()
{
	// Initilize the base time
	clock_gettime(CLOCK_MONOTONIC, &base_time);
}

//-----------------------------------------------------------------------------
uint64_t milliseconds()
{
	timespec tmp;

	clock_gettime(CLOCK_MONOTONIC, &tmp);

	return (tmp.tv_sec - base_time.tv_sec) * 1000 + (tmp.tv_nsec - base_time.tv_nsec) / 1000000;
}

//-----------------------------------------------------------------------------
uint64_t microseconds()
{
	timespec tmp;
	clock_gettime(CLOCK_MONOTONIC, &tmp);
	return (tmp.tv_sec - base_time.tv_sec) * 1000000 + (tmp.tv_nsec - base_time.tv_nsec) / 1000;
}

//-----------------------------------------------------------------------------
void* open_library(const char* path)
{
	void* library = dlopen(path, RTLD_NOW);

	if (library == NULL)
	{
		os::printf("OS: ERROR: Unable to load library '%s' with error: %s\n", path, dlerror());
		return NULL;
	}

	return library;
}

//-----------------------------------------------------------------------------
void close_library(void* library)
{
	CE_ASSERT(dlclose(library) == 0, "Failed to close library");
}

//-----------------------------------------------------------------------------
void* lookup_symbol(void* library, const char* name)
{
	dlerror();

	void* symbol = dlsym(library, name);

	const char* error = dlerror();

	if (error)
	{
		os::printf("OS: ERROR: Unable to lookup symbol '%s' with error: %s\n", name, error);
		return NULL;
	}

	return symbol;
}

//-----------------------------------------------------------------------------
void execute_process(const char* program, const char* params)
{
	int32_t ret;

	pid_t pid = fork();
	CE_ASSERT(pid != -1, "Unable to fork");

	if (pid)
	{
		wait(&ret);
	}
	else
	{
		int32_t res = execlp(program, program, params, NULL);
		CE_ASSERT(res != -1, "Unable to exec %s with error %d", program, res);
		exit(EXIT_SUCCESS);
	}
}


} // namespace os
} // namespace crown
