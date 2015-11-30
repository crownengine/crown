/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "log.h"
#include "console_server.h"
#include "string_utils.h"
#include "os.h"
#include "string_stream.h"

#if CROWN_DEBUG

namespace crown
{
namespace log_internal
{
	static StringStream& sanitize(StringStream& ss, const char* msg)
	{
		using namespace string_stream;
		const char* ch = msg;
		for (; *ch; ch++)
		{
			if (*ch == '"')
				ss << "\\";
			ss << *ch;
		}

		return ss;
	}

	static void console_log(const char* msg, LogSeverity::Enum severity)
	{
		using namespace string_stream;
		static const char* stt[] = { "info", "warning", "error", "debug" };

		// Build json message
		TempAllocator2048 alloc;
		StringStream json(alloc);

		json << "{\"type\":\"message\",";
		json << "\"severity\":\"" << stt[severity] << "\",";
		json << "\"message\":\""; sanitize(json, msg) << "\"}";

		console_server_globals::console().send(c_str(json));
	}

	void logx(LogSeverity::Enum sev, const char* msg, va_list args)
	{
		char buf[2048];
		int len = vsnprintf(buf, sizeof(buf), msg, args);
		if (len > (int)sizeof(buf))
			len = sizeof(buf) - 1;

		buf[len] = '\0';
		console_log(buf, sev);
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
