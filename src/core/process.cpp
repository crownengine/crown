/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/memory/temp_allocator.inl"
#include "core/process.h"
#include "core/strings/string_stream.inl"

#if CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#else
	#include <stdio.h>    // fdopen etc.
	#include <stdlib.h>   // exit
	#include <unistd.h>   // fork, execvp
	#include <sys/wait.h> // waitpid
	#include <errno.h>
#endif

namespace crown
{
struct Private
{
#if CROWN_PLATFORM_WINDOWS
	PROCESS_INFORMATION process;
	HANDLE stdout_rd;
	HANDLE stdout_wr;
#else
	FILE *file;
	pid_t pid;
#endif
};

namespace process_internal
{
	bool is_open(Private *priv)
	{
#if CROWN_PLATFORM_WINDOWS
		return priv->process.hProcess != 0;
#else
		return priv->pid != -1;
#endif
	}

} // namespace process_internal

Process::Process()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(*_priv));
	_priv = new (_data) Private();

#if CROWN_PLATFORM_WINDOWS
	memset(&_priv->process, 0, sizeof(_priv->process));
#else
	_priv->pid = -1;
#endif
}

Process::~Process()
{
	CE_ENSURE(process_internal::is_open(_priv) == false);
	_priv->~Private();
}

s32 Process::spawn(const char * const *argv, u32 flags)
{
	CE_ENSURE(process_internal::is_open(_priv) == false);

#if CROWN_PLATFORM_WINDOWS
	TempAllocator512 ta;
	StringStream path(ta);

	for (s32 i = 0; argv[i] != NULL; ++i) {
		const char *arg = argv[i];
		for (; *arg; ++arg) {
			if (*arg == ' ')
				break;
		}

		if (*arg)
			path << '"';

		path << argv[i];

		if (*arg)
			path << '"';

		path << ' ';
	}

	// https://docs.microsoft.com/it-it/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
	SECURITY_ATTRIBUTES sattr;
	sattr.nLength = sizeof(sattr);
	sattr.bInheritHandle = TRUE;
	sattr.lpSecurityDescriptor = NULL;

	if (flags & CROWN_PROCESS_STDOUT_PIPE) {
		// Pipe for STDOUT of child process
		BOOL ret;
		ret = CreatePipe(&_priv->stdout_rd, &_priv->stdout_wr, &sattr, 0);
		if (ret == 0)
			return -1;

		// Do not inherit read handle of STDOUT pipe
		ret = SetHandleInformation(_priv->stdout_rd, HANDLE_FLAG_INHERIT, 0);
		if (ret == 0)
			return -1;
	}

	STARTUPINFO info;
	memset(&info, 0, sizeof(info));
	info.cb = sizeof(info);
	info.hStdOutput = (flags & CROWN_PROCESS_STDOUT_PIPE) ? _priv->stdout_wr : 0;
	info.hStdError = (info.hStdOutput != 0) && (flags & CROWN_PROCESS_STDERR_MERGE) ? _priv->stdout_wr : 0;
	info.dwFlags |= (info.hStdOutput != 0 || info.hStdError != 0) ? STARTF_USESTDHANDLES : 0;

	BOOL err = CreateProcess(argv[0]
		, (LPSTR)string_stream::c_str(path)
		, NULL
		, NULL
		, TRUE // Handles are inherited
		, CREATE_NO_WINDOW
		, NULL
		, NULL
		, &info
		, &_priv->process
		);
	if (err == 0)
		return -1;

	if (flags & CROWN_PROCESS_STDOUT_PIPE)
		CloseHandle(_priv->stdout_wr);
	return 0;
#else
	// https://opensource.apple.com/source/Libc/Libc-167/gen.subproj/popen.c.auto.html
	int fildes[2];
	pid_t pid;

	if (flags & CROWN_PROCESS_STDIN_PIPE || flags & CROWN_PROCESS_STDOUT_PIPE) {
		if (pipe(fildes) < 0)
			return -1;
	}

	pid = fork();
	if (pid == -1) { // Error, cleanup and return
		close(fildes[0]);
		close(fildes[1]);
		return -1;
	} else if (pid == 0) { // Child
		if (flags & CROWN_PROCESS_STDOUT_PIPE) {
			if (fildes[1] != STDOUT_FILENO) {
				dup2(fildes[1], STDOUT_FILENO);
				close(fildes[1]);
				fildes[1] = STDOUT_FILENO;
			}
			close(fildes[0]);

			if (flags & CROWN_PROCESS_STDERR_MERGE) {
				dup2(fildes[1], 2);
			}
		} else if (flags & CROWN_PROCESS_STDIN_PIPE) {
			if (fildes[0] != STDIN_FILENO) {
				dup2(fildes[0], STDIN_FILENO);
				close(fildes[0]);
				fildes[0] = STDIN_FILENO;
			}
			close(fildes[1]);
		}

		execvp(argv[0], (char * const *)argv);
		exit(EXIT_FAILURE); // exec returned error
	}

	// Parent
	if (flags & CROWN_PROCESS_STDOUT_PIPE) {
		_priv->file = fdopen(fildes[0], "r");
		close(fildes[1]);
	} else if (flags & CROWN_PROCESS_STDIN_PIPE) {
		_priv->file = fdopen(fildes[1], "w");
		close(fildes[0]);
	} else {
		_priv->file = NULL;
	}

	_priv->pid = pid;
	return 0;
#endif // if CROWN_PLATFORM_WINDOWS
}

bool Process::spawned()
{
#if CROWN_PLATFORM_WINDOWS
	return _priv->process.hProcess != 0;
#else
	return _priv->pid != -1;
#endif
}

void Process::force_exit()
{
	CE_ENSURE(process_internal::is_open(_priv) == true);
#if CROWN_PLATFORM_WINDOWS
#else
	kill(_priv->pid, SIGKILL);
#endif
}

s32 Process::wait()
{
	CE_ENSURE(process_internal::is_open(_priv) == true);

#if CROWN_PLATFORM_WINDOWS
	DWORD exitcode = 1;
	::WaitForSingleObject(_priv->process.hProcess, INFINITE);
	GetExitCodeProcess(_priv->process.hProcess, &exitcode);
	CloseHandle(_priv->process.hProcess);
	CloseHandle(_priv->process.hThread);
	memset(&_priv->process, 0, sizeof(_priv->process));
	return (s32)exitcode;
#else
	pid_t pid;
	int wstatus;

	if (_priv->file != NULL) {
		fclose(_priv->file);
		_priv->file = NULL;
	}

	do {
		pid = waitpid(_priv->pid, &wstatus, 0);
	} while (pid == -1 && errno == EINTR);

	_priv->pid = -1;
	return WIFEXITED(wstatus) ? (s32)WEXITSTATUS(wstatus) : -1;
#endif // if CROWN_PLATFORM_WINDOWS
}

char *Process::read(u32 *num_bytes_read, char *data, u32 len)
{
	CE_ENSURE(process_internal::is_open(_priv) == true);
#if CROWN_PLATFORM_WINDOWS
	DWORD read;
	BOOL success = FALSE;

	success = ReadFile(_priv->stdout_rd, data, len, &read, NULL);
	if (!success) {
		*num_bytes_read = UINT32_MAX;
		return NULL;
	}

	if (read == 0) {
		// EOF
		*num_bytes_read = 0;
		return NULL;
	} else {
		*num_bytes_read = read;
		return data;
	}
#else
	CE_ENSURE(_priv->file != NULL);
	size_t read = fread(data, 1, len, _priv->file);
	if (read != len) {
		if (feof(_priv->file) != 0) {
			if (read == 0) {
				*num_bytes_read = 0;
				return NULL;
			}
		} else if (ferror(_priv->file) != 0) {
			*num_bytes_read = UINT32_MAX;
			return NULL;
		}
	}

	*num_bytes_read = read;
	return data;
#endif // if CROWN_PLATFORM_WINDOWS
}

} // namespace crown
