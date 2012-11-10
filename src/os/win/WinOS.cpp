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
	DWORD fileAttr;
	fileAttr = GetFileAttributes(path);
	return (fileAttr != INVALID_FILE_ATTRIBUTES);
}

//-----------------------------------------------------------------------------
bool OS::IsDir(const char* path)
{
	DWORD fileAttr;
	fileAttr = GetFileAttributes(path);
	return (fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

//-----------------------------------------------------------------------------
bool OS::IsReg(const char* path)
{
	return !IsDir(path);
}

//-----------------------------------------------------------------------------
bool OS::Mknod(const char* path)
{
	HANDLE hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(hFile);
	return true;
}

//-----------------------------------------------------------------------------
bool OS::Unlink(const char* path)
{
	return DeleteFile(path) == TRUE;
}

//-----------------------------------------------------------------------------
bool OS::Mkdir(const char* path)
{
	return CreateDirectory(path, NULL) == TRUE;
}

//-----------------------------------------------------------------------------
bool OS::Rmdir(const char* path)
{
	return RemoveDirectory(path) == TRUE;
}

//-----------------------------------------------------------------------------
const char* OS::GetCWD()
{
	static char cwdBuf[1024];
	int len = GetCurrentDirectory(1024, cwdBuf);

	if (len == 0)
	{
		return Str::EMPTY;
	}

	return cwdBuf;
}

//-----------------------------------------------------------------------------
const char* OS::GetHome()
{
	// TODO
	return Str::EMPTY;
}

//-----------------------------------------------------------------------------
const char* OS::GetEnv(const char* env)
{
	static char evnBuf[1024];
	int len = GetEnvironmentVariable(env, envBuf, 1024);

	if (len == 0)
	{
		return Str::EMPTY;
	}

	return envBuf;
}

//-----------------------------------------------------------------------------
bool OS::Ls(const char* path, List<Str>& fileList)
{
	return false; // TODO
}

} // namespace crown

