/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/platform.h"

#if CROWN_PLATFORM_LINUX && (CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG)

#include "core/strings/string.inl"
#include "core/strings/string_stream.inl"
#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h> // strchr
#include <unistd.h> // getpid

namespace crown
{
namespace error
{
	static const char* addr2line(const char* addr, char* line, int len)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "addr2line -s -e /proc/%u/exe %s", getpid(), addr);
		FILE* f = popen(buf, "r");
		if (f)
		{
			char* ret = fgets(line, len, f);
			CE_UNUSED(ret);
			line[strlen32(line) - 1] = '\0';
			pclose(f);
			return line;
		}
		return "<addr2line missing>";
	}

	void callstack(StringStream& ss)
	{
		void* array[64];
		int size = backtrace(array, countof(array));
		char** messages = backtrace_symbols(array, size);

		// skip first stack frame (points here)
		for (int i = 1; i < size && messages != NULL; ++i)
		{
			char* msg = messages[i];
			char* mangled_name = strchr(msg, '(');
			char* offset_begin = strchr(msg, '+');
			char* offset_end   = strchr(msg, ')');
			char* addr_begin   = strchr(msg, '[');
			char* addr_end     = strchr(msg, ']');

			char buf[512];

			// Attempt to demangle the symbol
			if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin)
			{
				*mangled_name++ = '\0';
				*offset_begin++ = '\0';
				*offset_end++   = '\0';
				*addr_begin++   = '\0';
				*addr_end++     = '\0';

				int demangle_ok;
				char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &demangle_ok);
				char line[256];
				memset(line, 0, sizeof(line));

				snprintf(buf
					, sizeof(buf)
					, "    [%2d] %s: (%s)+%s in %s\n"
					, i
					, msg
					, (demangle_ok == 0 ? real_name : mangled_name)
					, offset_begin
					, addr2line(addr_begin, line, sizeof(line))
					);

				free(real_name);
			}
			else
			{
				snprintf(buf, sizeof(buf), "    [%2d] %s\n", i, msg);
			}

			ss << buf;
		}
		free(messages);
	}

} // namespace error

} // namespace crown

#endif // CROWN_PLATFORM_LINUX && (CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG)
