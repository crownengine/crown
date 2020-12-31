/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/platform.h"
#include <stdint.h>
#include <stddef.h>

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

template <typename T>
inline void exchange(T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

template <typename T>
inline T min(const T& a, const T& b)
{
	return a < b ? a : b;
}

template <typename T>
inline T max(const T& a, const T& b)
{
	return a > b ? a : b;
}

template <typename T>
inline T clamp(T val, T mmin, T mmax)
{
	return min(max(mmin, val), mmax);
}

#if CROWN_DEBUG && !CROWN_DEVELOPMENT
	u32 STRING_ID_32(const char* str, const u32 id);
	u64 STRING_ID_64(const char* str, const u64 id);
#else
	#define STRING_ID_32(str, id) u32(id)
	#define STRING_ID_64(str, id) u64(id)
#endif
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

#define countof(arr) (sizeof(arr)/sizeof(arr[0]))
#define container_of(ptr, type, member) ((char*)ptr - offsetof(type, member))

#define CE_STRINGIZE(value) CE_STRINGIZE_(value)
#define CE_STRINGIZE_(value) #value

#define CE_NOOP(...) do { (void)0; } while (0)
#define CE_UNUSED(x) do { (void)(x); } while (0)
#define CE_STATIC_ASSERT(condition, ...) static_assert(condition, "" # __VA_ARGS__)

#if CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG
	#define CE_LIKELY(x) __builtin_expect((x), 1)
	#define CE_UNLIKELY(x) __builtin_expect((x), 0)
	#define CE_UNREACHABLE() __builtin_unreachable()
	#define CE_ALIGN_DECL(align, decl) decl __attribute__ ((aligned (align)))
	#define CE_THREAD __thread
#elif CROWN_COMPILER_MSVC
	#define CE_LIKELY(x) (x)
	#define CE_UNLIKELY(x) (x)
	#define CE_UNREACHABLE()
	#define CE_ALIGN_DECL(align_, decl) __declspec (align(align_)) decl
	#define CE_THREAD __declspec(thread)
#else
	#error "Unknown compiler"
#endif
