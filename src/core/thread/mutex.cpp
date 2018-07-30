/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.h"
#include "core/platform.h"
#include "core/thread/mutex.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <windows.h>
#endif

namespace crown
{
struct Private
{
#if CROWN_PLATFORM_POSIX
	pthread_mutex_t mutex;
#elif CROWN_PLATFORM_WINDOWS
	CRITICAL_SECTION cs;
#endif
};

Mutex::Mutex()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(Private));
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	pthread_mutexattr_t attr;
	int err = pthread_mutexattr_init(&attr);
	CE_ASSERT(err == 0, "pthread_mutexattr_init: errno = %d", err);
	err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	CE_ASSERT(err == 0, "pthread_mutexattr_settype: errno = %d", err);
	err = pthread_mutex_init(&priv->mutex, &attr);
	CE_ASSERT(err == 0, "pthread_mutex_init: errno = %d", err);
	err = pthread_mutexattr_destroy(&attr);
	CE_ASSERT(err == 0, "pthread_mutexattr_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	InitializeCriticalSection(&priv->cs);
#endif
}

Mutex::~Mutex()
{
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_destroy(&priv->mutex);
	CE_ASSERT(err == 0, "pthread_mutex_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	DeleteCriticalSection(&priv->cs);
#endif
}

void Mutex::lock()
{
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_lock(&priv->mutex);
	CE_ASSERT(err == 0, "pthread_mutex_lock: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	EnterCriticalSection(&priv->cs);
#endif
}

void Mutex::unlock()
{
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_unlock(&priv->mutex);
	CE_ASSERT(err == 0, "pthread_mutex_unlock: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	LeaveCriticalSection(&priv->cs);
#endif
}

void* Mutex::native_handle()
{
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	return &priv->mutex;
#elif CROWN_PLATFORM_WINDOWS
	return &priv->cs;
#endif
}

} // namespace crown
