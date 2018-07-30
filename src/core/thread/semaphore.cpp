/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.h"
#include "core/platform.h"
#include "core/thread/semaphore.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <windows.h>
	#include <limits.h> // LONG_MAX
#endif

namespace crown
{
struct Private
{
#if CROWN_PLATFORM_POSIX
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	s32 count;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE handle;
#endif
};

Semaphore::Semaphore()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(Private));
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	int err = 0;
	err = pthread_mutex_init(&priv->mutex, NULL);
	CE_ASSERT(err == 0, "pthread_mutex_init: errno = %d", err);
	err = pthread_cond_init(&priv->cond, NULL);
	CE_ASSERT(err == 0, "pthread_cond_init: errno = %d", err);
	CE_UNUSED(err);
	priv->count = 0;
#elif CROWN_PLATFORM_WINDOWS
	priv->handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	CE_ASSERT(priv->handle != NULL, "CreateSemaphore: GetLastError = %d", GetLastError());
	CE_UNUSED(priv->handle);
#endif
}

Semaphore::~Semaphore()
{
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_destroy(&priv->cond);
	CE_ASSERT(err == 0, "pthread_cond_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	BOOL err = CloseHandle(priv->handle);
	CE_ASSERT(err != 0, "CloseHandle: GetLastError = %d", GetLastError());
	CE_UNUSED(err);
#endif
}

void Semaphore::post(u32 count)
{
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	pthread_mutex_lock(&priv->mutex);
	for (u32 i = 0; i < count; ++i)
	{
		int err = pthread_cond_signal(&priv->cond);
		CE_ASSERT(err == 0, "pthread_cond_signal: errno = %d", err);
		CE_UNUSED(err);
	}

	priv->count += count;
	pthread_mutex_unlock(&priv->mutex);
#elif CROWN_PLATFORM_WINDOWS
	BOOL err = ReleaseSemaphore(priv->handle, count, NULL);
	CE_ASSERT(err != 0, "ReleaseSemaphore: GetLastError = %d", GetLastError());
	CE_UNUSED(err);
#endif
}

void Semaphore::wait()
{
	Private* priv = (Private*)_data;

#if CROWN_PLATFORM_POSIX
	pthread_mutex_lock(&priv->mutex);
	while (priv->count <= 0)
	{
		int err = pthread_cond_wait(&priv->cond, &priv->mutex);
		CE_ASSERT(err == 0, "pthread_cond_wait: errno = %d", err);
		CE_UNUSED(err);
	}
	priv->count--;
	pthread_mutex_unlock(&priv->mutex);
#elif CROWN_PLATFORM_WINDOWS
	DWORD err = WaitForSingleObject(priv->handle, INFINITE);
	CE_ASSERT(err == WAIT_OBJECT_0, "WaitForSingleObject: GetLastError = %d", GetLastError());
	CE_UNUSED(err);
#endif
}

} // namespace crown
