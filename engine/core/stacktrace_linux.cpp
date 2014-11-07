/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_LINUX && CROWN_COMPILER_GCC

#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>
#include <execinfo.h>

namespace crown
{

void stacktrace()
{
	void* array[50];
	int size = backtrace(array, 50);

	char** messages = backtrace_symbols(array, size);

	// skip first stack frame (points here)
	for (int i = 1; i < size && messages != NULL; ++i)
	{
		char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

		// find parantheses and +address offset surrounding mangled name
		for (char *p = messages[i]; *p; ++p)
		{
			if (*p == '(')
			{
				mangled_name = p;
			}
			else if (*p == '+')
			{
				offset_begin = p;
			}
			else if (*p == ')')
			{
				offset_end = p;
				break;
			}
		}

		// if the line could be processed, attempt to demangle the symbol
		if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin)
		{
			*mangled_name++ = '\0';
			*offset_begin++ = '\0';
			*offset_end++ = '\0';

			int status;
			char* real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

			printf("\t[%d] %s: (%s)+%s %s\n", i, messages[i], (status == 0 ? real_name : mangled_name), offset_begin, offset_end);
			free(real_name);
		}
		// otherwise, print the whole line
		else
		{
			printf("\t[%d] %s\n", i, messages[i]);
		}
	}
	free(messages);
}

} // namespace crown

#endif // CROWN_PLATFORM_LINUX && CROWN_COMPILER_GCC
