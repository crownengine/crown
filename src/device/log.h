/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
		const char *name;
	};

	void vlogx(LogSeverity::Enum sev, System system, const char *format, va_list args);
	void logx(LogSeverity::Enum sev, System system, const char *format, ...);

} // namespace log_internal

} // namespace crown

#define vlogi(system, format, va_list) crown::log_internal::vlogx(crown::LogSeverity::LOG_INFO, system, format, va_list)
#define vloge(system, format, va_list) crown::log_internal::vlogx(crown::LogSeverity::LOG_ERROR, system, format, va_list)
#define vlogw(system, format, va_list) crown::log_internal::vlogx(crown::LogSeverity::LOG_WARN, system, format, va_list)
#define logi(system, format, ...) crown::log_internal::logx(crown::LogSeverity::LOG_INFO, system, format, ## __VA_ARGS__)
#define loge(system, format, ...) crown::log_internal::logx(crown::LogSeverity::LOG_ERROR, system, format, ## __VA_ARGS__)
#define logw(system, format, ...) crown::log_internal::logx(crown::LogSeverity::LOG_WARN, system, format, ## __VA_ARGS__)

#define LOG_SYSTEM(ident, name) namespace { const crown::log_internal::System ident = { name }; }
