/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include <stdint.h>

#ifndef CROWN_DEBUG
	#define CROWN_DEBUG 0
#endif // CROWN_DEBUG

#ifndef CROWN_DEVELOPMENT
	#define CROWN_DEVELOPMENT 0
#endif // CROWN_DEVELOPMENT

/// @defgroup Core Core

namespace crown
{
/// @addtogroup Core
/// @{
typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
/// @}

} // namespace crown

#if defined(_MSC_VER)
	#define _ALLOW_KEYWORD_MACROS
#endif

#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif

#if !defined(__va_copy)
	#define __va_copy(dest, src) (dest = src)
#endif

#ifndef NULL
	#define NULL 0
#endif

#define countof(arr) (sizeof(arr)/sizeof(arr[0]))

#define CE_NOOP(...) do { (void)0; } while (0)
#define CE_UNUSED(x) do { (void)(x); } while (0)
#define CE_STATIC_ASSERT(condition, ...) static_assert(condition, "" # __VA_ARGS__)

#if defined(__GNUC__)
	#define CE_THREAD __thread
#elif defined(_MSC_VER)
	#define CE_THREAD __declspec(thread)
#else
	#error "Compiler not supported"
#endif
