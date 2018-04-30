/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/platform.h"
#include "core/thread/mutex.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <limits.h>
#endif

namespace crown
{
/// Semaphore.
///
/// @ingroup Thread.
struct Semaphore
{
#if CROWN_PLATFORM_POSIX
	Mutex _mutex;
	pthread_cond_t _cond;
	s32 _count;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE _handle;
#endif

	Semaphore();
	~Semaphore();
	Semaphore(const Semaphore&) = delete;
	Semaphore& operator=(const Semaphore&) = delete;

	///
	void post(u32 count = 1);

	///
	void wait();
};

} // namespace crown
