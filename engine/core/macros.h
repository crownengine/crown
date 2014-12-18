/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#ifdef _MSC_VER
	#define CE_ALIGNOF(x) __alignof(x)
	#define CE_EXPORT __declspec(dllexport)
	#define CE_INLINE __inline
	#define CE_THREAD __declspec(thread)
#elif defined __GNUG__
	#define CE_ALIGNOF(x) __alignof__(x)
	#define CE_EXPORT __attribute__ ((visibility("default")))
	#define CE_INLINE inline
	#define CE_THREAD __thread
#else
	#error "Compiler not supported"
#endif

#define CE_UNUSED(x) do { (void)(x); } while (0)

#define CE_COUNTOF(arr) (sizeof(arr)/sizeof(arr[0]))
