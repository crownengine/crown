/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "config.h"

#if CROWN_PLATFORM_WINDOWS
	#include "types.h"
	#include "win_headers.h"
#endif

namespace crown
{

struct AtomicInt
{
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
		InterlockedExchangeAdd(&_val, (int32_t)0);
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

private:

#if CROWN_PLATFORM_POSIX && CROWN_COMPILER_GCC
	mutable int _val;
#elif CROWN_PLATFORM_WINDOWS
	mutable LONG _val;
#endif
};

} // namespace crown
