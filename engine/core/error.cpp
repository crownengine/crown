/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "error.h"
#include "stacktrace.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

namespace crown
{
namespace error
{
	void abort(const char* file, int line, const char* message, ...)
	{
		va_list ap;
		va_start(ap, message);
		vprintf(message, ap);
		va_end(ap);
		printf("\tIn: %s:%d\n", file, line);
		printf("Stacktrace:\n");
		stacktrace();
		exit(EXIT_FAILURE);
	}
} // namespace error
} // namespace crown
