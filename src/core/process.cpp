/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/memory/temp_allocator.h"
#include "core/process.h"
#include "core/strings/string_stream.h"

#if CROWN_PLATFORM_POSIX
	#include <unistd.h>   // fork, execv
#elif CROWN_PLATFORM_WINDOWS
	#include <windows.h>
#endif

namespace crown
{
struct Private
{
#if CROWN_PLATFORM_POSIX
	FILE* file;
#elif CROWN_PLATFORM_WINDOWS
	PROCESS_INFORMATION process;
#endif
};

namespace process_internal
{
	bool is_open(Private* priv)
	{
#if CROWN_PLATFORM_POSIX
		return priv->file != NULL;
#elif CROWN_PLATFORM_WINDOWS
		return priv->process.hProcess != 0;
#endif
	}
}

Process::Process()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(Private));
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	priv->file = NULL;
#elif CROWN_PLATFORM_WINDOWS
	memset(&priv->process, 0, sizeof(priv->process));
#endif
}

Process::~Process()
{
	Private* priv = (Private*)_data;
	CE_ENSURE(process_internal::is_open(priv) == false);
}

s32 Process::spawn(const char* const* argv)
{
	Private* priv = (Private*)_data;
	CE_ENSURE(process_internal::is_open(priv) == false);

	TempAllocator512 ta;
	StringStream path(ta);

	path << argv[0];
	path << ' ';
#if CROWN_PLATFORM_POSIX
	path << "2>&1 ";
#endif
	for (s32 i = 1; argv[i] != NULL; ++i)
	{
		const char* arg = argv[i];
		for (; *arg; ++arg)
		{
			if (*arg == ' ')
				path << '\\';
			path << *arg;
		}
		path << ' ';
	}
#if CROWN_PLATFORM_POSIX
	priv->file = popen(string_stream::c_str(path), "r");

	return priv->file != NULL ? 0 : -1;
#elif CROWN_PLATFORM_WINDOWS
	STARTUPINFO info;
	memset(&info, 0, sizeof(info));
	info.cb = sizeof(info);

	int err = CreateProcess(argv[0]
		, (LPSTR)string_stream::c_str(path)
		, NULL
		, NULL
		, FALSE
		, CREATE_NO_WINDOW
		, NULL
		, NULL
		, &info
		, &priv->process
		);
	return (s32)err;
#endif
}

s32 Process::wait(StringStream* output)
{
	Private* priv = (Private*)_data;
	CE_ENSURE(process_internal::is_open(priv) == true);

#if CROWN_PLATFORM_POSIX
	if (output != NULL)
	{
		char buf[1024];
		while (fgets(buf, sizeof(buf), priv->file) != NULL)
			*output << buf;
	}

	s32 exitcode = pclose(priv->file);
	priv->file = NULL;
	return exitcode;
#elif CROWN_PLATFORM_WINDOWS
	DWORD exitcode = 1;
	::WaitForSingleObject(priv->process.hProcess, INFINITE);
	GetExitCodeProcess(priv->process.hProcess, &exitcode);
	CloseHandle(priv->process.hProcess);
	CloseHandle(priv->process.hThread);
	memset(priv->process, 0, sizeof(priv->process));
	return (s32)exitcode;
#endif
}

} // namespace crown
