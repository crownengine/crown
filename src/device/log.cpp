/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/platform.h"
#include "core/strings/string.inl"
#include "core/strings/string_stream.h"
#include "core/thread/scoped_mutex.inl"
#include "device/console_server.h"
#include "device/log.h"

namespace crown
{
namespace log_internal
{
	static Mutex s_mutex;

	static void stdout_log(LogSeverity::Enum sev, System system, const char* msg)
	{
		char buf[8192];
#if CROWN_PLATFORM_POSIX
		#define ANSI_RESET  "\x1b[0m"
		#define ANSI_YELLOW "\x1b[33m"
		#define ANSI_RED    "\x1b[31m"

		const char* stt[] = { ANSI_RESET, ANSI_YELLOW, ANSI_RED };
		CE_STATIC_ASSERT(countof(stt) == LogSeverity::COUNT);

		snprintf(buf, sizeof(buf), "%s%s: %s\n" ANSI_RESET, stt[sev], system.name, msg);
#else
		snprintf(buf, sizeof(buf), "%s: %s\n", system.name, msg);
#endif
		os::log(buf);
	}

	void vlogx(LogSeverity::Enum sev, System system, const char* msg, va_list args)
	{
		ScopedMutex sm(s_mutex);

		char buf[8192];
		vsnprintf(buf, sizeof(buf), msg, args);

		stdout_log(sev, system, buf);

		if (console_server())
			console_server()->log(sev, system.name, buf);
	}

	void logx(LogSeverity::Enum sev, System system, const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		vlogx(sev, system, msg, args);
		va_end(args);
	}

} // namespace log

} // namespace crown
