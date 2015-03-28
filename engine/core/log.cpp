/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "log.h"
#include "console_server.h"
#include "string_utils.h"
#include "os.h"

#if CROWN_DEBUG

namespace crown
{
namespace log_internal
{
	void logx(LogSeverity::Enum sev, const char* msg, va_list args)
	{
		char buf[2048];
		int len = vsnprintf(buf, sizeof(buf), msg, args);
		if (len > (int)sizeof(buf))
			len = sizeof(buf) - 1;

		buf[len] = '\0';
		console_server_globals::console().log(buf, sev);
		os::log(buf);
	}

	void logx(LogSeverity::Enum sev, const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		logx(sev, msg, args);
		va_end(args);
	}
} // namespace log
} // namespace crown

#endif // CROWN_DEBUG
