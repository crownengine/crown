/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/callstack.h"
#include "core/error/error.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/string_stream.h"
#include "device/log.h"
#include <stdarg.h>
#include <stdlib.h> // exit

namespace { const crown::log_internal::System ERROR = { "Error" }; }

namespace crown
{
namespace error
{
	static void abort(const char* format, va_list args)
	{
		char buf[1024];
		vsnprintf(buf, sizeof(buf), format, args);

		TempAllocator4096 ta;
		StringStream ss(ta);
		ss << buf;
		ss << "Stacktrace:\n";
		callstack(ss);

		loge(ERROR, string_stream::c_str(ss));
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
