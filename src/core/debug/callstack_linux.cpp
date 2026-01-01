/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_LINUX && (CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG)
#include "core/debug/callstack.h"
#include "core/process.h"
#include "core/strings/string.inl"
#include <cxxabi.h>   // __cxa_demangle
#include <execinfo.h> // backtrace, backtrace_symbols_fd
#include <stdio.h>    // fdopen, fgets
#include <stdlib.h>   // free
#include <string.h>   // strchr
#include <sys/wait.h> // waitpid
#include <unistd.h>   // getpid
#include <stb_sprintf.h>

namespace crown
{
namespace debug
{
	static int symbol_fds[2];
	static int demangled_fds[2];
	static int exit_fds[2];
	static pid_t demangler;

	static const char *addr2line(char *line, int len, const char *addr)
	{
		char process_exe[256];
		stbsp_snprintf(process_exe, sizeof(process_exe), "/proc/%u/exe", getpid());

		const char *argv[] =
		{
			"addr2line",
			"-s",
			"-e",
			process_exe,
			addr,
			NULL
		};

		Process pr;
		if (pr.spawn(argv, CROWN_PROCESS_STDOUT_PIPE | CROWN_PROCESS_STDERR_MERGE) == 0) {
			u32 num_read;
			pr.read(&num_read, line, len);
			line[num_read - 1] = '\0';
			pr.wait();
			return line;
		}
		return "<addr2line missing>";
	}

	static int demangler_main()
	{
		FILE *fp = fdopen(symbol_fds[0], "r");
		if (fp == NULL)
			return EXIT_FAILURE;

		while (true) {
			fd_set fdset;

			FD_ZERO(&fdset);
			FD_SET(symbol_fds[0], &fdset);
			FD_SET(exit_fds[0], &fdset);
			int maxfd = max(symbol_fds[0], exit_fds[0]);

			if (select(maxfd + 1, &fdset, NULL, NULL, NULL) <= 0)
				continue;

			if (FD_ISSET(exit_fds[0], &fdset)) {
				break;
			} else if (FD_ISSET(symbol_fds[0], &fdset)) {
				char msg[512];
				char buf[512];

				while (fgets(msg, sizeof(msg), fp) != NULL) {
					u32 len = strlen32(msg);
					if (len <= 1)
						break;
					msg[len - 1] = '\0';

					char *mangled_name = strchr(msg, '(');
					char *offset_begin = strchr(msg, '+');
					char *offset_end   = strchr(msg, ')');
					char *addr_begin   = strchr(msg, '[');
					char *addr_end     = strchr(msg, ']');

					// Attempt to demangle the symbol.
					if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin) {
						*mangled_name++ = '\0';
						*offset_begin++ = '\0';
						*offset_end++   = '\0';
						*addr_begin++   = '\0';
						*addr_end++     = '\0';

						int demangle_ok;
						char *real_name = abi::__cxa_demangle(mangled_name, 0, 0, &demangle_ok);
						char line[256];
						memset(line, 0, sizeof(line));

						stbsp_snprintf(buf
							, sizeof(buf)
							, "%s(%s+%s) in %s"
							, msg
							, (demangle_ok == 0 ? real_name : mangled_name)
							, offset_begin
							, addr2line(line, sizeof(line), addr_begin)
							);

						free(real_name);
					} else {
						stbsp_snprintf(buf, sizeof(buf), "%s", msg);
					}

					write(demangled_fds[1], buf, sizeof(buf));
				}
			}
		}

		fclose(fp);
		return EXIT_SUCCESS;
	}

	void callstack_shutdown()
	{
		int wstatus;

		if (demangler <= 0)
			return;

		write(exit_fds[1], "q", 1);
		waitpid(demangler, &wstatus, 0);
		demangler = 0;
	}

	s32 callstack_init()
	{
		s32 ret = -1;

		if (pipe(symbol_fds) < 0)
			goto close_0;
		if (pipe(demangled_fds) < 0)
			goto close_1;
		if (pipe(exit_fds) < 0)
			goto close_2;

		demangler = fork();
		if (demangler == -1) {
			close(symbol_fds[0]);
			close(symbol_fds[1]);
			close(demangled_fds[0]);
			close(demangled_fds[1]);
			close(exit_fds[0]);
			close(exit_fds[1]);
			return -1;
		} else if (demangler == 0) {
			// Block all signals.
			sigset_t set;
			sigfillset(&set);
			sigprocmask(SIG_BLOCK, &set, NULL);

			close(symbol_fds[1]);
			close(demangled_fds[0]);
			close(exit_fds[1]);
			exit(demangler_main());
		} else {
			ret = 0;
		}

		close(exit_fds[0]);
	close_2:
		close(demangled_fds[1]);
	close_1:
		close(symbol_fds[0]);
	close_0:
		return ret;
	}

	void callstack(log_internal::System system, LogSeverity::Enum severity)
	{
		char msg[512] = {};
		void *array[64];

		if (demangler <= 0) {
			log_internal::logx(severity, system, "Callstack unavailable.");
			return;
		}

		// Get symbols and write them to demangler process.
		int size = backtrace(array, countof(array));
		backtrace_symbols_fd(array, size, symbol_fds[1]);
		write(symbol_fds[1], "\n", 1);

		// Log demangled symbols.
		for (int i = 0; i < size; ++i) {
			if (read(demangled_fds[0], msg, sizeof(msg)) <= 0)
				break;
			if (i >= 1) // Skip this very function.
				log_internal::logx(severity, system, "[%2d] %s", i, msg);
		}
	}

} // namespace debug

} // namespace crown

#endif // if CROWN_PLATFORM_LINUX && (CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG)
