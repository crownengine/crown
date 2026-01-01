/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/error/error.h"

#if CROWN_DEBUG
	#define CE_ASSERT(condition, msg, ...)                   \
		do                                                   \
		{                                                    \
			if (CE_UNLIKELY(!(condition)))                   \
			{                                                \
				crown::error::abort("Assertion failed: %s\n" \
					"    In: %s:%d\n"                        \
					"    " msg                               \
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
#endif // if CROWN_DEBUG

#define CE_FATAL(msg, ...) CE_ASSERT(false, msg, ## __VA_ARGS__)
#define CE_ENSURE(condition) CE_ASSERT(condition, "")
