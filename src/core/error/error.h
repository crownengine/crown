/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

/// @defgroup Error Error
/// @ingroup Core
namespace crown
{
/// Error management.
///
/// @ingroup Error
namespace error
{
	/// Aborts the program execution logging an error message and the stacktrace if
	/// the platform supports it.
	void abort(const char* format, ...);

} // namespace error

} // namespace crown

#if CROWN_DEBUG
	#define CE_ASSERT(condition, msg, ...)                   \
		do                                                   \
		{                                                    \
			if (CE_UNLIKELY(!(condition)))                   \
			{                                                \
				crown::error::abort("Assertion failed: %s\n" \
					"    In: %s:%d\n"                        \
					"    " msg "\n"                          \
					, # condition                            \
					, __FILE__                               \
					, __LINE__                               \
					, ## __VA_ARGS__                         \
					);                                       \
				CE_UNREACHABLE();                            \
			}                                                \
		} while (0)
#else
	#define CE_ASSERT(...) CE_NOOP()
#endif // CROWN_DEBUG

#define CE_FATAL(msg, ...) CE_ASSERT(false, msg, ## __VA_ARGS__)
#define CE_ENSURE(condition) CE_ASSERT(condition, "")
