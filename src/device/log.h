/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include <stdarg.h>

namespace crown
{
/// Enumerates log levels.
///
/// @ingroup Device
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

#define logiv(msg, va_list) crown::log_internal::logx(crown::LogSeverity::INFO, msg, va_list)
#define logdv(msg, va_list) crown::log_internal::logx(crown::LogSeverity::DEBUG, msg, va_list)
#define logev(msg, va_list) crown::log_internal::logx(crown::LogSeverity::ERROR, msg, va_list)
#define logwv(msg, va_list) crown::log_internal::logx(crown::LogSeverity::WARN, msg, va_list)
#define logi(msg, ...) crown::log_internal::logx(crown::LogSeverity::INFO, msg, ##__VA_ARGS__)
#define logd(msg, ...) crown::log_internal::logx(crown::LogSeverity::DEBUG, msg, ##__VA_ARGS__)
#define loge(msg, ...) crown::log_internal::logx(crown::LogSeverity::ERROR, msg, ##__VA_ARGS__)
#define logw(msg, ...) crown::log_internal::logx(crown::LogSeverity::WARN, msg, ##__VA_ARGS__)
