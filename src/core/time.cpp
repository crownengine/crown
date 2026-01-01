/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/time.h"

#if CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#elif CROWN_PLATFORM_OSX
	#include <sys/time.h> // gettimeofday
#else
	#include <time.h> // clock_gettime
#endif

namespace crown
{
namespace time
{
	s64 now()
	{
#if CROWN_PLATFORM_WINDOWS
		LARGE_INTEGER ttime;
		QueryPerformanceCounter(&ttime);
		return (s64)ttime.QuadPart;
#elif CROWN_PLATFORM_OSX
		struct timeval now;
		gettimeofday(&now, NULL);
		return now.tv_sec * s64(1000000) + now.tv_usec;
#else
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * s64(1000000000) + now.tv_nsec;
#endif
	}

	inline s64 frequency()
	{
#if CROWN_PLATFORM_WINDOWS
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (s64)freq.QuadPart;
#elif CROWN_PLATFORM_OSX
		return s64(1000000);
#else
		return s64(1000000000);
#endif
	}

	f64 seconds(s64 ticks)
	{
		return f64(ticks) / f64(frequency());
	}

} // namespace time

} // namespace crown
