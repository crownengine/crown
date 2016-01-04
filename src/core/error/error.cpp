/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "error.h"
#include "stacktrace.h"
#include "log.h"
#include <stdlib.h>
#include <stdarg.h>

namespace crown
{
namespace error
{
	static void abort(const char* file, int line, const char* format, va_list args)
	{
		CE_LOGEV(format, args);
		CE_LOGE("\tIn: %s:%d", file, line);
		CE_LOGE("Stacktrace:");
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
