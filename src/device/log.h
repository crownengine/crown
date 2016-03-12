/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

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
		DEBUG,

		COUNT
	};
};

namespace log_internal
{
	void logx(LogSeverity::Enum sev, const char* msg, va_list args);
	void logx(LogSeverity::Enum sev, const char* msg, ...);
} // namespace log_internal
} // namespace crown

#define CE_LOGIV(msg, va_list) crown::log_internal::logx(crown::LogSeverity::INFO, msg, va_list)
#define CE_LOGDV(msg, va_list) crown::log_internal::logx(crown::LogSeverity::DEBUG, msg, va_list)
#define CE_LOGEV(msg, va_list) crown::log_internal::logx(crown::LogSeverity::ERROR, msg, va_list)
#define CE_LOGWV(msg, va_list) crown::log_internal::logx(crown::LogSeverity::WARN, msg, va_list)
#define CE_LOGI(msg, ...) crown::log_internal::logx(crown::LogSeverity::INFO, msg, ##__VA_ARGS__)
#define CE_LOGD(msg, ...) crown::log_internal::logx(crown::LogSeverity::DEBUG, msg, ##__VA_ARGS__)
#define CE_LOGE(msg, ...) crown::log_internal::logx(crown::LogSeverity::ERROR, msg, ##__VA_ARGS__)
#define CE_LOGW(msg, ...) crown::log_internal::logx(crown::LogSeverity::WARN, msg, ##__VA_ARGS__)
