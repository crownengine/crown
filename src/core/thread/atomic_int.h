/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/platform.h"

#if CROWN_PLATFORM_WINDOWS
	#include "core/types.h"
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

namespace crown
{
/// Atomic integer.
///
/// @ingroup Thread
struct AtomicInt
{
#if CROWN_PLATFORM_POSIX && CROWN_COMPILER_GCC
	mutable int _val;
#elif CROWN_PLATFORM_WINDOWS
	mutable LONG _val;
#endif

	AtomicInt(int val)
	{
		store(val);
	}

	int load() const
	{
#if CROWN_PLATFORM_POSIX && CROWN_COMPILER_GCC
		__sync_fetch_and_add(&_val, 0);
		return _val;
#elif CROWN_PLATFORM_WINDOWS
		InterlockedExchangeAdd(&_val, (s32)0);
		return _val;
#endif
	}

	void store(int val)
	{
#if CROWN_PLATFORM_POSIX && CROWN_COMPILER_GCC
		__sync_lock_test_and_set(&_val, val);
#elif CROWN_PLATFORM_WINDOWS
		InterlockedExchange(&_val, val);
#endif
	}
};

} // namespace crown
