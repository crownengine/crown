/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/platform.h"
#include "core/strings/string.inl"
#include "core/strings/string_stream.inl"
#include "core/thread/scoped_mutex.inl"
#include "device/log.h"
#if CROWN_LOG_TO_CONSOLE
	#include "device/console_server.h"
#endif
#include <stb_sprintf.h>

namespace crown
{
namespace log_internal
{
	static Mutex s_mutex;

	static void stdout_log(LogSeverity::Enum sev, System system, const char *format)
	{
		char buf[8192];
#if CROWN_PLATFORM_POSIX && !CROWN_PLATFORM_ANDROID && !CROWN_PLATFORM_EMSCRIPTEN
	#define ANSI_RESET  "\x1b[0m"
	#define ANSI_YELLOW "\x1b[33m"
	#define ANSI_RED    "\x1b[31m"

		const char *stt[] = { ANSI_RESET, ANSI_YELLOW, ANSI_RED };
		CE_STATIC_ASSERT(countof(stt) == LogSeverity::COUNT);

		stbsp_snprintf(buf, sizeof(buf), "%s%s: %s\n" ANSI_RESET, stt[sev], system.name, format);
#else
		stbsp_snprintf(buf, sizeof(buf), "%s: %s\n", system.name, format);
#endif
		os::log(buf);
	}

	/// Sends a log message to all clients.
	static void console_log(LogSeverity::Enum sev, System system, const char *msg)
	{
#if !CROWN_LOG_TO_CONSOLE
		CE_UNUSED_3(sev, system, msg);
#else
		if (!console_server())
			return;

		const char *severity_map[] = { "info", "warning", "error" };
		CE_STATIC_ASSERT(countof(severity_map) == LogSeverity::COUNT);

		TempAllocator4096 ta;
		StringStream ss(ta);

		ss << "{\"type\":\"message\",\"severity\":\"";
		ss << severity_map[sev];
		ss << "\",\"system\":\"";
		ss << system.name;
		ss << "\",\"message\":\"";

		// Sanitize msg
		const char *ch = msg;
		for (; *ch; ch++) {
			if (*ch == '"' || *ch == '\\')
				ss << "\\";
			ss << *ch;
		}
		ss << "\"}";

		console_server()->broadcast(string_stream::c_str(ss));
#endif // if !CROWN_LOG_TO_CONSOLE
	}

	void vlogx(LogSeverity::Enum sev, System system, const char *format, va_list args)
	{
		ScopedMutex sm(s_mutex);

		char buf[8192];
		stbsp_vsnprintf(buf, sizeof(buf), format, args);

		stdout_log(sev, system, buf);
		console_log(sev, system, buf);
	}

	void logx(LogSeverity::Enum sev, System system, const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		vlogx(sev, system, format, args);
		va_end(args);
	}

} // namespace log

} // namespace crown
