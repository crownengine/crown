/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "platform.h"

#ifdef CROWN_COMPILER_MSVC
	#define CE_ALIGNOF(x) __alignof(x)
	#define CE_EXPORT __declspec(dllexport)
	#define CE_INLINE __inline
	#define CE_THREAD __declspec(thread)
#elif CROWN_COMPILER_GCC
	#define CE_ALIGNOF(x) __alignof__(x)
	#define CE_EXPORT __attribute__ ((visibility("default")))
	#define CE_INLINE inline
	#define CE_THREAD __thread
#else
	#error "Compiler not supported"
#endif

#define CE_UNUSED(x) do { (void)(x); } while (0)
#define CE_COUNTOF(arr) (sizeof(arr)/sizeof(arr[0]))
#define CE_CONCATENATE1(a, b) a ## b
#define CE_CONCATENATE(a, b) CE_CONCATENATE1(a, b)
#define CE_STATIC_ASSERT(condition) typedef int CE_CONCATENATE(STATIC_ASSERT,__LINE__)[condition ? 1 : -1]
