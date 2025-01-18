/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_LINUX && (CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG)
#include "core/error/callstack.h"
#include "core/process.h"
#include "core/strings/string.inl"
#include <cxxabi.h>
#include <execinfo.h>
#include <stb_sprintf.h>
#include <stdlib.h> // free
#include <string.h> // strchr
#include <unistd.h> // getpid

namespace crown
{
namespace error
{
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

	void callstack(log_internal::System system, LogSeverity::Enum severity)
	{
		void *array[64];
		int size = backtrace(array, countof(array));
		char **messages = backtrace_symbols(array, size);

		// skip first stack frame (points here)
		for (int i = 1; i < size && messages != NULL; ++i) {
			char *msg = messages[i];
			char *mangled_name = strchr(msg, '(');
			char *offset_begin = strchr(msg, '+');
			char *offset_end   = strchr(msg, ')');
			char *addr_begin   = strchr(msg, '[');
			char *addr_end     = strchr(msg, ']');

			// Attempt to demangle the symbol
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

				log_internal::logx(severity
					, system
					, "[%2d] %s: (%s)+%s in %s"
					, i
					, msg
					, (demangle_ok == 0 ? real_name : mangled_name)
					, offset_begin
					, addr2line(line, sizeof(line), addr_begin)
					);

				free(real_name);
			} else {
				log_internal::logx(severity, system, "[%2d] %s", i, msg);
			}
		}
		free(messages);
	}

} // namespace error

} // namespace crown

#endif // if CROWN_PLATFORM_LINUX && (CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG)
