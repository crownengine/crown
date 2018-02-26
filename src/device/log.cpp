/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/memory/temp_allocator.h"
#include "core/os.h"
#include "core/platform.h"
#include "core/strings/string.h"
#include "core/strings/string_stream.h"
#include "core/thread/mutex.h"
#include "device/console_server.h"
#include "device/device.h"
#include "device/log.h"

namespace crown
{
namespace log_internal
{
	static Mutex s_mutex;

	void logxv(LogSeverity::Enum sev, System system, const char* msg, va_list args)
	{
		ScopedMutex sm(s_mutex);

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

		if (console_server())
		{
			static const char* s_severity_map[] = { "info", "warning", "error" };
			CE_STATIC_ASSERT(countof(s_severity_map) == LogSeverity::COUNT);

			TempAllocator4096 ta;
			StringStream json(ta);

			json << "{\"type\":\"message\",";
			json << "\"severity\":\"";
			json << s_severity_map[sev];
			json << "\",";
			json << "\"system\":\"";
			json << system.name;
			json << "\",";
			json << "\"message\":\"";

			// Sanitize buf
			const char* ch = buf;
			for (; *ch; ch++)
			{
				if (*ch == '"' || *ch == '\\')
					json << "\\";
				json << *ch;
			}

			json << "\"}";

			console_server()->send(string_stream::c_str(json));
		}

		if (device())
			device()->log(buf);
	}

	void logx(LogSeverity::Enum sev, System system, const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		logxv(sev, system, msg, args);
		va_end(args);
	}

} // namespace log

} // namespace crown
