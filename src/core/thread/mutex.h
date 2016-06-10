/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "platform.h"
#include "types.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include "win_headers.h"
#endif

namespace crown
{
/// Mutex.
///
/// @ingroup Thread
struct Mutex
{
#if CROWN_PLATFORM_POSIX
	pthread_mutex_t _mutex;
#elif CROWN_PLATFORM_WINDOWS
	CRITICAL_SECTION _cs;
#endif

	Mutex()
	{
#if CROWN_PLATFORM_POSIX
		pthread_mutexattr_t attr;
		int err = pthread_mutexattr_init(&attr);
		CE_ASSERT(err == 0, "pthread_mutexattr_init: errno = %d", err);
		err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		CE_ASSERT(err == 0, "pthread_mutexattr_settype: errno = %d", err);
		err = pthread_mutex_init(&_mutex, &attr);
		CE_ASSERT(err == 0, "pthread_mutex_init: errno = %d", err);
		err = pthread_mutexattr_destroy(&attr);
		CE_ASSERT(err == 0, "pthread_mutexattr_destroy: errno = %d", err);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		InitializeCriticalSection(&_cs);
#endif
	}

	~Mutex()
	{
#if CROWN_PLATFORM_POSIX
		int err = pthread_mutex_destroy(&_mutex);
		CE_ASSERT(err == 0, "pthread_mutex_destroy: errno = %d", err);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		DeleteCriticalSection(&_cs);
#endif
	}

	/// Locks the mutex.
	void lock()
	{
#if CROWN_PLATFORM_POSIX
		int err = pthread_mutex_lock(&_mutex);
		CE_ASSERT(err == 0, "pthread_mutex_lock: errno = %d", err);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		EnterCriticalSection(&_cs);
#endif
	}

	/// Unlocks the mutex.
	void unlock()
	{
#if CROWN_PLATFORM_POSIX
		int err = pthread_mutex_unlock(&_mutex);
		CE_ASSERT(err == 0, "pthread_mutex_unlock: errno = %d", err);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		LeaveCriticalSection(&_cs);
#endif
	}

private:

	// Disable copying.
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&);
};

/// Automatically locks a mutex when created and unlocks when destroyed.
///
/// @ingroup Thread
struct ScopedMutex
{
	Mutex& _mutex;

	/// Locks the mutex @a m.
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

	// Disable copying
	ScopedMutex(const ScopedMutex&);
	ScopedMutex& operator=(const ScopedMutex&);
};

} // namespace crown
