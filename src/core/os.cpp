/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "dynamic_string.h"
#include "os.h"
#include "string_stream.h"
#include "temp_allocator.h"
#include "vector.h"
#include <string.h> // strcmp

#if CROWN_PLATFORM_POSIX
	#include <dirent.h> // opendir, readdir
#endif

namespace crown
{
namespace os
{
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
			vector::push_back(files, filename);
		}
		while (FindNextFile(file, &ffd) != 0);

		FindClose(file);
#endif
	}

	int execute_process(const char* path, const char* args, StringStream& output)
	{
#if CROWN_PLATFORM_POSIX
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

		int err = CreateProcess(path, (LPSTR)args, NULL, NULL, TRUE, 0, NULL, NULL, &info, &process);
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
