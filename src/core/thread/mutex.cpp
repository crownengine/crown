/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/platform.h"
#include "core/thread/mutex.h"
#include <new>

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
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(*_priv));
	_priv = new (_data) Private();

#if CROWN_PLATFORM_POSIX
	pthread_mutexattr_t attr;
	int err = pthread_mutexattr_init(&attr);
	CE_ASSERT(err == 0, "pthread_mutexattr_init: errno = %d", err);
	err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	CE_ASSERT(err == 0, "pthread_mutexattr_settype: errno = %d", err);
	err = pthread_mutex_init(&_priv->mutex, &attr);
	CE_ASSERT(err == 0, "pthread_mutex_init: errno = %d", err);
	err = pthread_mutexattr_destroy(&attr);
	CE_ASSERT(err == 0, "pthread_mutexattr_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	InitializeCriticalSection(&_priv->cs);
#endif
}

Mutex::~Mutex()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_destroy(&_priv->mutex);
	CE_ASSERT(err == 0, "pthread_mutex_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	DeleteCriticalSection(&_priv->cs);
#endif
	_priv->~Private();
}

void Mutex::lock()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_lock(&_priv->mutex);
	CE_ASSERT(err == 0, "pthread_mutex_lock: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	EnterCriticalSection(&_priv->cs);
#endif
}

void Mutex::unlock()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_mutex_unlock(&_priv->mutex);
	CE_ASSERT(err == 0, "pthread_mutex_unlock: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	LeaveCriticalSection(&_priv->cs);
#endif
}

void* Mutex::native_handle()
{
#if CROWN_PLATFORM_POSIX
	return &_priv->mutex;
#elif CROWN_PLATFORM_WINDOWS
	return &_priv->cs;
#endif
}

} // namespace crown
