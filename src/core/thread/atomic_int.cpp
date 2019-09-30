/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/platform.h"
#include "core/thread/atomic_int.h"

#if CROWN_PLATFORM_WINDOWS
	#include <windows.h>
#endif

namespace crown
{
AtomicInt::AtomicInt(s32 val)
	: _val(val)
{
}

s32 AtomicInt::load()
{
#if CROWN_PLATFORM_POSIX && (CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG)
	__sync_fetch_and_add(&_val, 0);
	return _val;
#elif CROWN_PLATFORM_WINDOWS
	InterlockedExchangeAdd((LONG*)&_val, (s32)0);
	return _val;
#endif
}

void AtomicInt::store(s32 val)
{
#if CROWN_PLATFORM_POSIX && (CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG)
	__sync_lock_test_and_set(&_val, val);
#elif CROWN_PLATFORM_WINDOWS
	InterlockedExchange((LONG*)&_val, val);
#endif
}

} // namespace crown
