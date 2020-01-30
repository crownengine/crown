/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/memory/temp_allocator.inl"
#include "core/process.h"
#include "core/strings/string_stream.inl"

#if CROWN_PLATFORM_POSIX
	#include <unistd.h>   // fork, execvp
	#include <sys/wait.h> // waitpid
	#include <errno.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <windows.h>
#endif

namespace crown
{
struct Private
{
#if CROWN_PLATFORM_POSIX
	FILE* file;
	pid_t pid;
#elif CROWN_PLATFORM_WINDOWS
	PROCESS_INFORMATION process;
#endif
};

namespace process_internal
{
	bool is_open(Private* priv)
	{
#if CROWN_PLATFORM_POSIX
		return priv->pid != -1;
#elif CROWN_PLATFORM_WINDOWS
		return priv->process.hProcess != 0;
#endif
	}
}

Process::Process()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(*_priv));
	_priv = new (_data) Private();

#if CROWN_PLATFORM_POSIX
	_priv->pid = -1;
#elif CROWN_PLATFORM_WINDOWS
	memset(&_priv->process, 0, sizeof(_priv->process));
#endif
}

Process::~Process()
{
	CE_ENSURE(process_internal::is_open(_priv) == false);
	_priv->~Private();
}

s32 Process::spawn(const char* const* argv, u32 flags)
{
	CE_ENSURE(process_internal::is_open(_priv) == false);

#if CROWN_PLATFORM_POSIX
	// https://opensource.apple.com/source/Libc/Libc-167/gen.subproj/popen.c.auto.html
	int fildes[2];
	pid_t pid;

	if (flags & ProcessFlags::STDIN_PIPE || flags & ProcessFlags::STDOUT_PIPE)
	{
		if (pipe(fildes) < 0)
			return -1;
	}

	pid = fork();
	if (pid == -1) // Error, cleanup and return
	{
		close(fildes[0]);
		close(fildes[1]);
		return -1;
	}
	else if (pid == 0) // Child
	{
		if (flags & ProcessFlags::STDOUT_PIPE)
		{
			if (fildes[1] != STDOUT_FILENO)
			{
				dup2(fildes[1], STDOUT_FILENO);
				close(fildes[1]);
				fildes[1] = STDOUT_FILENO;
			}
			close(fildes[0]);

			if (flags & ProcessFlags::STDERR_MERGE)
			{
				dup2(fildes[1], 2);
			}
		}
		else if (flags & ProcessFlags::STDIN_PIPE)
		{
			if (fildes[0] != STDIN_FILENO)
			{
				dup2(fildes[0], STDIN_FILENO);
				close(fildes[0]);
				fildes[0] = STDIN_FILENO;
			}
			close(fildes[1]);
		}

		execvp(argv[0], (char* const*)argv);
		// exec returned error
		return -1;
	}

	// Parent
	if (flags & ProcessFlags::STDOUT_PIPE)
	{
		_priv->file = fdopen(fildes[0], "r");
		close(fildes[1]);
	}
	else if (flags & ProcessFlags::STDIN_PIPE)
	{
		_priv->file = fdopen(fildes[1], "w");
		close(fildes[0]);
	}
	else
	{
		_priv->file = NULL;
	}

	_priv->pid = pid;
	return 0;
#elif CROWN_PLATFORM_WINDOWS
	TempAllocator512 ta;
	StringStream path(ta);

	for (s32 i = 0; argv[i] != NULL; ++i)
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
		, &_priv->process
		);
	return (s32)(err != 0 ? 0 : -err);
#endif
}

bool Process::spawned()
{
#if CROWN_PLATFORM_POSIX
	return _priv->pid != -1;
#elif CROWN_PLATFORM_WINDOWS
	return _priv->process.hProcess != 0;
#endif
}

void Process::force_exit()
{
	CE_ENSURE(process_internal::is_open(_priv) == true);
#if CROWN_PLATFORM_POSIX
	kill(_priv->pid, SIGKILL);
#elif CROWN_PLATFORM_WINDOWS
#endif
}

s32 Process::wait()
{
	CE_ENSURE(process_internal::is_open(_priv) == true);

#if CROWN_PLATFORM_POSIX
	pid_t pid;
	int wstatus;

	if (_priv->file != NULL)
	{
		fclose(_priv->file);
		_priv->file = NULL;
	}

	do
	{
		pid = waitpid(_priv->pid, &wstatus, 0);
	}
	while (pid == -1 && errno == EINTR);

	_priv->pid = -1;
	return WIFEXITED(wstatus) ? (s32)WEXITSTATUS(wstatus) : -1;
#elif CROWN_PLATFORM_WINDOWS
	DWORD exitcode = 1;
	::WaitForSingleObject(_priv->process.hProcess, INFINITE);
	GetExitCodeProcess(_priv->process.hProcess, &exitcode);
	CloseHandle(_priv->process.hProcess);
	CloseHandle(_priv->process.hThread);
	memset(&_priv->process, 0, sizeof(_priv->process));
	return (s32)exitcode;
#endif
}

char* Process::fgets(char* data, u32 len)
{
	CE_ENSURE(process_internal::is_open(_priv) == true);
#if CROWN_PLATFORM_POSIX
	CE_ENSURE(_priv->file != NULL);
	char* ret = ::fgets(data, len, _priv->file);
	return ret;
#elif CROWN_PLATFORM_WINDOWS
	return NULL;
#endif
}

} // namespace crown
