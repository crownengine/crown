/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#if defined(CROWN_DEBUG)

#include "console_server.h"
#include "string_utils.h"
#include "os.h"

namespace crown
{
namespace log_internal
{
	inline void logx(LogSeverity::Enum sev, const char* msg, va_list args)
	{
		char buf[2048];
		int len = vsnprintf(buf, sizeof(buf), msg, args);
		if (len > (int)sizeof(buf))
			len = sizeof(buf) - 1;

		buf[len] = '\0';
		console_server_globals::console().log(buf, sev);
		os::log(buf);
	}

	inline void logx(LogSeverity::Enum sev, const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		logx(sev, msg, args);
		va_end(args);
	}
} // namespace log_internal
} // namespace crown
	#define CE_LOGI(msg, ...) crown::log_internal::logx(crown::LogSeverity::INFO, msg, ##__VA_ARGS__)
	#define CE_LOGD(msg, ...) crown::log_internal::logx(crown::LogSeverity::DEBUG, msg, ##__VA_ARGS__)
	#define CE_LOGE(msg, ...) crown::log_internal::logx(crown::LogSeverity::ERROR, msg, ##__VA_ARGS__)
	#define CE_LOGW(msg, ...) crown::log_internal::logx(crown::LogSeverity::WARN, msg, ##__VA_ARGS__)
#else
	#define CE_LOGI(msg, ...) ((void)0)
	#define CE_LOGD(msg, ...) ((void)0)
	#define CE_LOGE(msg, ...) ((void)0)
	#define CE_LOGW(msg, ...) ((void)0)
#endif // defined(CROWN_DEBUG)
