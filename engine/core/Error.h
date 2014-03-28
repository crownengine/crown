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

#include "Log.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#if defined __GNUG__ && defined(LINUX)
	#include <cxxabi.h>
	#include <execinfo.h>
#endif

#include "Config.h"

#pragma once

namespace crown
{
namespace error
{

inline void log_backtrace()
{
	#if defined __GNUG__ && defined(LINUX)
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

			Log::e("\t[%d] %s: (%s)+%s %s", i, messages[i], (status == 0 ? real_name : mangled_name), offset_begin, offset_end);
			free(real_name);
		}
		// otherwise, print the whole line
		else
		{
			Log::e("\t[%d] %s", i, messages[i]);
		}
	}
	free(messages);
	#endif
}

/// Aborts the program execution logging an error message and the stacktrace if
/// the platform supports it.
inline void abort(const char* file, int line, const char* message, ...)
{
	va_list ap;
	va_start(ap, message);
	Log::e1(message, ap);
	va_end(ap);
	Log::e("\tIn: %s:%d\n", file, line);
	Log::e("Backtrace:");
	//fflush(0);
	log_backtrace();
	exit(EXIT_FAILURE);
}

} // namespace error
} // namespace crown