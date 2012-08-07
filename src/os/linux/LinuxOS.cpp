/*
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

#include "OS.h"
#include <cstdio>
#include <cstdarg>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

namespace Crown
{

const char OS::PATH_SEPARATOR = '/';

//-----------------------------------------------------------------------------
void OS::Printf(const char* string, ...)
{
	va_list args;

	va_start(args, string);
	vprintf(string, args);
	va_end(args);
}

//-----------------------------------------------------------------------------
void OS::Vprintf(const char* string, va_list arg)
{
	vprintf(string, arg);
}

//-----------------------------------------------------------------------------
void OS::LogDebug(const char* string, va_list arg)
{
	Printf("D: ");
	Vprintf(string, arg);
	Printf("\n");
}

//-----------------------------------------------------------------------------
void OS::LogError(const char* string, va_list arg)
{
	Printf("E: ");
	Vprintf(string, arg);
	Printf("\n");
}

//-----------------------------------------------------------------------------
void OS::LogWarning(const char* string, va_list arg)
{
	Printf("W: ");
	Vprintf(string, arg);
	Printf("\n");
}

//-----------------------------------------------------------------------------
void OS::LogInfo(const char* string, va_list arg)
{
	Printf("I: ");
	Vprintf(string, arg);
	Printf("\n");
}

//-----------------------------------------------------------------------------
bool OS::Exists(const char* path)
{
	struct stat dummy;
	return (stat(path, &dummy) == 0);
}

//-----------------------------------------------------------------------------
bool OS::IsDir(const char* path)
{
	struct stat info;
	memset(&info, 0, sizeof(struct stat));
	lstat(path, &info);
	return ((S_ISDIR(info.st_mode)) != 0 && (S_ISLNK(info.st_mode) == 0));
}

//-----------------------------------------------------------------------------
bool OS::IsReg(const char* path)
{
	struct stat info;
	memset(&info, 0, sizeof(struct stat));
	lstat(path, &info);
	return ((S_ISREG(info.st_mode) != 0) && (S_ISLNK(info.st_mode) == 0));
}

//-----------------------------------------------------------------------------
bool OS::Mknod(const char* path)
{
	// Permission mask: rw-r--r--
	return mknod(path, S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0) == 0;
}

//-----------------------------------------------------------------------------
bool OS::Unlink(const char* path)
{
	return (unlink(path) == 0);
}

//-----------------------------------------------------------------------------
bool OS::Mkdir(const char* path)
{
	// rwxr-xr-x permission mask
	return (mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0);
}

//-----------------------------------------------------------------------------
bool OS::Rmdir(const char* path)
{
	return (rmdir(path) == 0);
}

//-----------------------------------------------------------------------------
const char* OS::GetCWD()
{
	static char cwdBuf[1024];
	if (getcwd(cwdBuf, 1024) == NULL)
	{
		return Str::EMPTY;
	}

	return cwdBuf;
}

//-----------------------------------------------------------------------------
const char* OS::GetHome()
{
	char* envHome = NULL;
	envHome = getenv("HOME");

	if (envHome == NULL)
	{
		return Str::EMPTY;
	}

	return envHome;
}

//-----------------------------------------------------------------------------
const char* OS::GetEnv(const char* env)
{
	char* envDevel = NULL;
	envDevel = getenv(env);

	if (envDevel == NULL)
	{
		return Str::EMPTY;
	}

	return envDevel;
}

//-----------------------------------------------------------------------------
bool OS::Ls(const char* path, List<Str>& fileList)
{
	DIR *dir;
	struct dirent *ent;

	dir = opendir(path);

	if (dir == NULL)
	{
		return false;
	}

	while ((ent = readdir (dir)) != NULL)
	{
		fileList.Append(Str(ent->d_name));
	}

	closedir (dir);

	return true;
}

} // namespace Crown

