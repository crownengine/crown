/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/error/error.h"
#include "core/platform.h"
#include "core/types.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
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

	Mutex();

	~Mutex();

	/// Locks the mutex.
	void lock();

	/// Unlocks the mutex.
	void unlock();

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
