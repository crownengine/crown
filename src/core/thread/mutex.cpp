/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "mutex.h"

namespace crown
{
Mutex::Mutex()
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

Mutex::~Mutex()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_destroy(&_mutex);
	CE_ASSERT(err == 0, "pthread_mutex_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	DeleteCriticalSection(&_cs);
#endif
}

void Mutex::lock()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_lock(&_mutex);
	CE_ASSERT(err == 0, "pthread_mutex_lock: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	EnterCriticalSection(&_cs);
#endif
}

void Mutex::unlock()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_unlock(&_mutex);
	CE_ASSERT(err == 0, "pthread_mutex_unlock: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	LeaveCriticalSection(&_cs);
#endif
}

} // namespace crown

