/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
		LOG_INFO,
		LOG_WARN,
		LOG_ERROR,

		COUNT
	};
};

namespace log_internal
{
	struct System
	{
		const char* name;
	};

	void vlogx(LogSeverity::Enum sev, System system, const char* msg, va_list args);
	void logx(LogSeverity::Enum sev, System system, const char* msg, ...);

} // namespace log_internal

} // namespace crown

#define vlogi(system, msg, va_list) crown::log_internal::vlogx(crown::LogSeverity::LOG_INFO, system, msg, va_list)
#define vloge(system, msg, va_list) crown::log_internal::vlogx(crown::LogSeverity::LOG_ERROR, system, msg, va_list)
#define vlogw(system, msg, va_list) crown::log_internal::vlogx(crown::LogSeverity::LOG_WARN, system, msg, va_list)
#define logi(system, msg, ...) crown::log_internal::logx(crown::LogSeverity::LOG_INFO, system, msg, ## __VA_ARGS__)
#define loge(system, msg, ...) crown::log_internal::logx(crown::LogSeverity::LOG_ERROR, system, msg, ## __VA_ARGS__)
#define logw(system, msg, ...) crown::log_internal::logx(crown::LogSeverity::LOG_WARN, system, msg, ## __VA_ARGS__)

#define LOG_SYSTEM(ident, name) namespace { const crown::log_internal::System ident = { name }; }
