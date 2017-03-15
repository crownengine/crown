/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "mutex.h"
#include "platform.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
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

	void post(u32 count = 1);

	void wait();

private:

	// Disable copying
	Semaphore(const Semaphore& s);
	Semaphore& operator=(const Semaphore& s);
};

} // namespace crown
