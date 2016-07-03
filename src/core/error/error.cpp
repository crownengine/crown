/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "error.h"
#include "log.h"
#include <stdarg.h>
#include <stdlib.h> // exit

namespace crown
{
namespace error
{
	static void abort(const char* file, int line, const char* format, va_list args)
	{
		logev(format, args);
		loge("\tIn: %s:%d\n\nStacktrace:", file, line);
		print_callstack();
		exit(EXIT_FAILURE);
	}

	void abort(const char* file, int line, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		abort(file, line, format, args);
		va_end(args);
	}
} // namespace error
} // namespace crown
