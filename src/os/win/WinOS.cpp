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
#include <windows.h>
#include <cstdio>
#include <cstdarg>

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
bool exists(const char* path)
{
	DWORD fileAttr;
	fileAttr = GetFileAttributes(path);
	return (fileAttr != INVALID_FILE_ATTRIBUTES);
}

//-----------------------------------------------------------------------------
bool is_dir(const char* path)
{
	DWORD fileAttr;
	fileAttr = GetFileAttributes(path);
	return (fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

//-----------------------------------------------------------------------------
bool is_reg(const char* path)
{
	return !IsDir(path);
}

//-----------------------------------------------------------------------------
bool mknod(const char* path)
{
	HANDLE hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(hFile);
	return true;
}

//-----------------------------------------------------------------------------
bool unlink(const char* path)
{
	return DeleteFile(path) == TRUE;
}

//-----------------------------------------------------------------------------
bool mkdir(const char* path)
{
	return CreateDirectory(path, NULL) == TRUE;
}

//-----------------------------------------------------------------------------
bool OS::rmdir(const char* path)
{
	return RemoveDirectory(path) == TRUE;
}

//-----------------------------------------------------------------------------
const char* get_cwd()
{
	static char cwdBuf[1024];
	int32_t len = GetCurrentDirectory(1024, cwdBuf);

	if (len == 0)
	{
		return Str::EMPTY;
	}

	return cwdBuf;
}

//-----------------------------------------------------------------------------
const char* get_home()
{
	// TODO
	return Str::EMPTY;
}

//-----------------------------------------------------------------------------
const char* get_env(const char* env)
{
	static char evnBuf[1024];
	int32_t len = GetEnvironmentVariable(env, envBuf, 1024);

	if (len == 0)
	{
		return Str::EMPTY;
	}

	return envBuf;
}

//-----------------------------------------------------------------------------
bool ls(const char* path, List<Str>& fileList)
{
	// TODO
	return false; 
}

} // namespace crown

