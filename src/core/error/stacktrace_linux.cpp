/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_LINUX && CROWN_COMPILER_GCC

#include "macros.h"
#include "string_utils.h"
#include "log.h"
#include <stdlib.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <string.h> // strchr
#include <unistd.h> // getpid

namespace crown
{
const char* addr2line(const char* addr, char* line, int len)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "addr2line -e /proc/%u/exe %s", getpid(), addr);
	FILE* f = popen(buf, "r");
	if (f)
	{
		fgets(line, len, f);
		line[strlen32(line) - 1] = '\0';
		pclose(f);
		return line;
	}
	return "<addr2line missing>";
}

void print_callstack()
{
	void* array[64];
	int size = backtrace(array, CE_COUNTOF(array));
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

		// if the line could be processed, attempt to demangle the symbol
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

			CE_LOGE("\t[%2d] %s: (%s)+%s in %s"
				, i
				, msg
				, (demangle_ok == 0 ? real_name : mangled_name)
				, offset_begin
				, addr2line(addr_begin, line, sizeof(line))
				);

			free(real_name);
		}
		// otherwise, print the whole line
		else
		{
			CE_LOGE("\t[%2d] %s", i, msg);
		}
	}
	free(messages);
}

} // namespace crown

#endif // CROWN_PLATFORM_LINUX && CROWN_COMPILER_GCC
