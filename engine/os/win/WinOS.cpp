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

#include "OS.h"
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include "StringUtils.h"

namespace crown
{
namespace os
{

// FIXME: timespec does not exists in win
// static timespec base_time;
	LARGE_INTEGER frequency;
	LARGE_INTEGER base_time;

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
		if ((path[0] >= 65 && path[0] <= 90) || (path[0] >= 97 && path[0] <= 122))
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
		if ((path[0] >= 'c' && path[0] <= 'z') || (path[0] >= 'C' && path[0] <= 'Z'))
		{
			if (path[1] == ':')
			{
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool exists(const char* path)
{
	DWORD fileAttr;
	fileAttr = GetFileAttributes(path);
	return (fileAttr != INVALID_FILE_ATTRIBUTES);
}

//-----------------------------------------------------------------------------
bool is_directory(const char* path)
{
	DWORD fileAttr;
	fileAttr = GetFileAttributes(path);
	return (fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

//-----------------------------------------------------------------------------
bool is_file(const char* path)
{
	return !is_directory(path);
}

//-----------------------------------------------------------------------------
bool create_file(const char* path)
{
	HANDLE hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(hFile);
	return true;
}

//-----------------------------------------------------------------------------
bool delete_file(const char* path)
{
	return DeleteFile(path) == TRUE;
}

//-----------------------------------------------------------------------------
bool create_directory(const char* path)
{
	return CreateDirectory(path, NULL) == TRUE;
}

//-----------------------------------------------------------------------------
bool delete_directory(const char* path)
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
		return string::EMPTY;
	}

	return cwdBuf;
}

//-----------------------------------------------------------------------------
const char* get_home()
{
	// TODO
	return string::EMPTY;
}

//-----------------------------------------------------------------------------
const char* get_env(const char* env)
{
	static char envBuf[1024];
	int32_t len = GetEnvironmentVariable(env, envBuf, 1024);

	if (len == 0)
	{
		return string::EMPTY;
	}

	return envBuf;
}

//-----------------------------------------------------------------------------
void list_files(const char* path, Vector<DynamicString>& files)
{
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
		files.push_back(filename);
	}
	while (FindNextFile(file, &ffd) != 0);

	FindClose(file);
}

//-----------------------------------------------------------------------------
const char* normalize_path(const char* path)
{
	static char norm[1024];

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
}

//-----------------------------------------------------------------------------
void init_os()
{
	QueryPerformanceFrequency(&frequency);

	CE_ASSERT(frequency.QuadPart > 0, "Hardware does not support high resolution performance counter.\n");
	
	QueryPerformanceCounter(&base_time);
}

//-----------------------------------------------------------------------------
uint64_t milliseconds()
{
	LARGE_INTEGER current_time;

	QueryPerformanceCounter(&current_time);

	return (uint64_t) (current_time.QuadPart - base_time.QuadPart) / (frequency.QuadPart / 1000);
}

//-----------------------------------------------------------------------------
uint64_t microseconds()
{
	LARGE_INTEGER current_time;

	QueryPerformanceCounter(&current_time);

	return (uint64_t) (current_time.QuadPart - base_time.QuadPart) / (frequency.QuadPart / 1000000);
}

//-----------------------------------------------------------------------------
void* open_library(const char* path)
{

	HMODULE library = LoadLibrary(path);

	CE_ASSERT(library  != NULL, "Unable to load library '%s' with error: %d\n", path, GetLastError());

	return library;
}

//-----------------------------------------------------------------------------
void close_library(void* library)
{
	BOOL freed = FreeLibrary((HMODULE)library);

	CE_ASSERT(freed,  "Failed to close library\n with error: %d\n", GetLastError());
}

//-----------------------------------------------------------------------------
void* lookup_symbol(void* library, const char* name)
{
	FARPROC symbol = GetProcAddress((HMODULE)library, name);

	CE_ASSERT(symbol  != NULL, "Unable to export symbol '%s' with error: %d\n", name, GetLastError());
	return symbol;
}

//-----------------------------------------------------------------------------
void execute_process(const char* args[])
{
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

	char tmp[1024];
	string::strncpy(tmp, normalize_path(cmds.c_str()), string::strlen(cmds.c_str()));
	printf("normalized: %s\n", tmp);

	printf("current dir: %s\n", get_cwd());
	
	int32_t res;
	if (res = CreateProcess(args[0], tmp, NULL, NULL, TRUE, 0, NULL, NULL, &info, &process))
	{
	    ::WaitForSingleObject(process.hProcess, INFINITE);
   		CloseHandle(process.hProcess);
    	CloseHandle(process.hThread);
	}
	else
	{
		printf("Unable to create process for %s, errno: %d\n\n", args[0], GetLastError());
	}
}

} // namespace os
} // namespace crown