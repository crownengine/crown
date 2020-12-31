/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/time.h"

#if CROWN_PLATFORM_POSIX
	#include <time.h> // clock_gettime
#elif CROWN_PLATFORM_WINDOWS
	#include <windows.h>
#endif

namespace crown
{
namespace time
{
	s64 now()
	{
#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_ANDROID
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * s64(1000000000) + now.tv_nsec;
#elif CROWN_PLATFORM_OSX
		struct timeval now;
		gettimeofday(&now, NULL);
		return now.tv_sec * s64(1000000) + now.tv_usec;
#elif CROWN_PLATFORM_WINDOWS
		LARGE_INTEGER ttime;
		QueryPerformanceCounter(&ttime);
		return (s64)ttime.QuadPart;
#endif
	}

	inline s64 frequency()
	{
#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_ANDROID
		return s64(1000000000);
#elif CROWN_PLATFORM_OSX
		return s64(1000000);
#elif CROWN_PLATFORM_WINDOWS
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		return (s64)freq.QuadPart;
#endif
	}

	f64 seconds(s64 ticks)
	{
		return f64(ticks) / f64(frequency());
	}

} // namespace time

} // namespace crown
