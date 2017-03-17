/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "callstack.h"
#include "error.h"
#include "log.h"
#include "string_stream.h"
#include "temp_allocator.h"
#include <stdarg.h>
#include <stdlib.h> // exit

namespace { const crown::log_internal::System ERROR = { "Error" }; }

namespace crown
{
namespace error
{
	static void abort(const char* format, va_list args)
	{
		logev(ERROR, format, args);

		TempAllocator4096 ta;
		StringStream ss(ta);
		callstack(ss);
		loge(ERROR, "Stacktrace:\n%s", string_stream::c_str(ss));
		exit(EXIT_FAILURE);
	}

	void abort(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		abort(format, args);
		va_end(args);
	}
} // namespace error
} // namespace crown
