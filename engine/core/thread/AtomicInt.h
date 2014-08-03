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

#include "Config.h"

#if CROWN_PLATFORM_WINDOWS
	#include "Types.h"
	#include "WinHeaders.h"
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
		__sync_fetch_and_add(&m_val, 0);
		return m_val;
		#elif CROWN_PLATFORM_WINDOWS
		InterlockedExchangeAdd(&m_val, (int32_t)0);
		return m_val;
		#endif
	}

	void store(int val)
	{
		#if CROWN_PLATFORM_POSIX && CROWN_COMPILER_GCC
		__sync_lock_test_and_set(&m_val, val);
		#elif CROWN_PLATFORM_WINDOWS
		InterlockedExchange(&m_val, val);
		#endif
	}

private:

	#if CROWN_PLATFORM_POSIX && CROWN_COMPILER_GCC
	mutable int m_val;
	#elif CROWN_PLATFORM_WINDOWS
	mutable LONG m_val;
	#endif
};

} // namespace crown
