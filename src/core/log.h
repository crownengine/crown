/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include <stdarg.h>

namespace crown
{

/// Enumerates log levels.
struct LogSeverity
{
	enum Enum
	{
		INFO,
		WARN,
		ERROR,
		DEBUG
	};
};

namespace log_internal
{
	void logx(LogSeverity::Enum sev, const char* msg, va_list args);
	void logx(LogSeverity::Enum sev, const char* msg, ...);
} // namespace log_internal
} // namespace crown

#if CROWN_DEBUG
	#define CE_LOGI(msg, ...) crown::log_internal::logx(crown::LogSeverity::INFO, msg, ##__VA_ARGS__)
	#define CE_LOGD(msg, ...) crown::log_internal::logx(crown::LogSeverity::DEBUG, msg, ##__VA_ARGS__)
	#define CE_LOGE(msg, ...) crown::log_internal::logx(crown::LogSeverity::ERROR, msg, ##__VA_ARGS__)
	#define CE_LOGW(msg, ...) crown::log_internal::logx(crown::LogSeverity::WARN, msg, ##__VA_ARGS__)
#else
	#define CE_LOGI(msg, ...) ((void)0)
	#define CE_LOGD(msg, ...) ((void)0)
	#define CE_LOGE(msg, ...) ((void)0)
	#define CE_LOGW(msg, ...) ((void)0)
#endif // CROWN_DEBUG
