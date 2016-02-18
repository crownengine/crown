/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "console_server.h"
#include "log.h"
#include "os.h"
#include "platform.h"
#include "string_stream.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "device.h"

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

	static void console_log(const char* msg, LogSeverity::Enum sev)
	{
		if (!console_server_globals::console())
			return;

		static const char* stt[] = { "info", "warning", "error", "debug" };

		// Build json message
		using namespace string_stream;
		TempAllocator4096 ta;
		StringStream json(ta);

		json << "{\"type\":\"message\",";
		json << "\"severity\":\"" << stt[sev] << "\",";
		json << "\"message\":\""; sanitize(json, msg) << "\"}";

		console_server_globals::console()->send(c_str(json));
	}

	void logx(LogSeverity::Enum sev, const char* msg, va_list args)
	{
		char buf[8192];
		int len = vsnprintf(buf, sizeof(buf), msg, args);
		buf[len] = '\0';

#if CROWN_PLATFORM_POSIX
		#define ANSI_RESET  "\x1b[0m"
		#define ANSI_YELLOW "\x1b[33m"
		#define ANSI_RED    "\x1b[31m"

		static const char* stt[] =
		{
			ANSI_RESET,
			ANSI_YELLOW,
			ANSI_RED,
			ANSI_RESET
		};

		os::log(stt[sev]);
		os::log(buf);
		os::log(ANSI_RESET);
#else
		os::log(buf);
#endif
		os::log("\n");

		if (device())
		{
			device()->log(buf);
		}

		console_log(buf, sev);
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
