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
#include "types.h"
#include "assert.h"
#include "macros.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include "win_headers.h"
#endif

namespace crown
{

struct Mutex
{
	Mutex()
	{
#if CROWN_PLATFORM_POSIX
		int result = pthread_mutexattr_init(&_attr);
		CE_ASSERT(result == 0, "pthread_mutexattr_init: errno = %d", result);
		result = pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_ERRORCHECK);
		CE_ASSERT(result == 0, "pthread_mutexattr_settype: errno = %d", result);
		result = pthread_mutex_init(&_mutex, &_attr);
		CE_ASSERT(result == 0, "pthread_mutex_init: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		InitializeCriticalSection(&_cs);
#endif
	}

	~Mutex()
	{
#if CROWN_PLATFORM_POSIX
		int result = pthread_mutex_destroy(&_mutex);
		CE_ASSERT(result == 0, "pthread_mutex_destroy: errno = %d", result);
		result = pthread_mutexattr_destroy(&_attr);
		CE_ASSERT(result == 0, "pthread_mutexattr_destroy: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		DeleteCriticalSection(&_cs);
#endif

	}

	void lock()
	{
#if CROWN_PLATFORM_POSIX
		int result = pthread_mutex_lock(&_mutex);
		CE_ASSERT(result == 0, "pthread_mutex_lock: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		EnterCriticalSection(&_cs);
#endif
	}

	void unlock()
	{
#if CROWN_PLATFORM_POSIX
		int result = pthread_mutex_unlock(&_mutex);
		CE_ASSERT(result == 0, "pthread_mutex_unlock: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		LeaveCriticalSection(&_cs);
#endif
	}

public:

#if CROWN_PLATFORM_POSIX
	pthread_mutex_t _mutex;
	pthread_mutexattr_t _attr;
#elif CROWN_PLATFORM_WINDOWS
	CRITICAL_SECTION _cs;
#endif

private:

	// Disable copying.
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&);
};

/// Automatically locks a mutex when created and unlocks when destroyed.
struct ScopedMutex
{
	/// Locks the given @a m mutex.
	ScopedMutex(Mutex& m)
		: _mutex(m)
	{
		_mutex.lock();
	}

	/// Unlocks the mutex passed to ScopedMutex::ScopedMutex()
	~ScopedMutex()
	{
		_mutex.unlock();
	}

private:

	Mutex& _mutex;

private:

	// Disable copying
	ScopedMutex(const ScopedMutex&);
	ScopedMutex& operator=(const ScopedMutex&);
};

} // namespace crown
