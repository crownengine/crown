/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/platform.h"
#include "core/thread/semaphore.h"
#include <new>

#if CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <limits.h> // LONG_MAX
#else
	#include <pthread.h>
#endif

namespace crown
{
struct Private
{
#if CROWN_PLATFORM_WINDOWS
	HANDLE handle;
#else
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	s32 count;
#endif
};

Semaphore::Semaphore()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(*_priv));
	_priv = new (_data) Private();

#if CROWN_PLATFORM_WINDOWS
	_priv->handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	CE_ASSERT(_priv->handle != NULL, "CreateSemaphore: GetLastError = %d", GetLastError());
	CE_UNUSED(_priv->handle);
#else
	int err = 0;
	err = pthread_mutex_init(&_priv->mutex, NULL);
	CE_ASSERT(err == 0, "pthread_mutex_init: errno = %d", err);
	err = pthread_cond_init(&_priv->cond, NULL);
	CE_ASSERT(err == 0, "pthread_cond_init: errno = %d", err);
	CE_UNUSED(err);
	_priv->count = 0;
#endif
}

Semaphore::~Semaphore()
{
#if CROWN_PLATFORM_WINDOWS
	BOOL err = CloseHandle(_priv->handle);
	CE_ASSERT(err != 0, "CloseHandle: GetLastError = %d", GetLastError());
	CE_UNUSED(err);
#else
	int err = pthread_cond_destroy(&_priv->cond);
	CE_ASSERT(err == 0, "pthread_cond_destroy: errno = %d", err);
	CE_UNUSED(err);
#endif

	_priv->~Private();
}

void Semaphore::post(u32 count)
{
#if CROWN_PLATFORM_WINDOWS
	BOOL err = ReleaseSemaphore(_priv->handle, count, NULL);
	CE_ASSERT(err != 0, "ReleaseSemaphore: GetLastError = %d", GetLastError());
	CE_UNUSED(err);
#else
	pthread_mutex_lock(&_priv->mutex);
	for (u32 i = 0; i < count; ++i) {
		int err = pthread_cond_signal(&_priv->cond);
		CE_ASSERT(err == 0, "pthread_cond_signal: errno = %d", err);
		CE_UNUSED(err);
	}

	_priv->count += count;
	pthread_mutex_unlock(&_priv->mutex);
#endif
}

void Semaphore::wait()
{
#if CROWN_PLATFORM_WINDOWS
	DWORD err = WaitForSingleObject(_priv->handle, INFINITE);
	CE_ASSERT(err == WAIT_OBJECT_0, "WaitForSingleObject: GetLastError = %d", GetLastError());
	CE_UNUSED(err);
#else
	pthread_mutex_lock(&_priv->mutex);
	while (_priv->count <= 0) {
		int err = pthread_cond_wait(&_priv->cond, &_priv->mutex);
		CE_ASSERT(err == 0, "pthread_cond_wait: errno = %d", err);
		CE_UNUSED(err);
	}
	_priv->count--;
	pthread_mutex_unlock(&_priv->mutex);
#endif
}

bool Semaphore::try_wait()
{
#if CROWN_PLATFORM_WINDOWS
	DWORD err = WaitForSingleObject(_priv->handle, 0u);
	if (err == WAIT_OBJECT_0)
		return true;
	else if (err == WAIT_TIMEOUT)
		return false;
	CE_FATAL("WaitForSingleObject: GetLastError = %d", GetLastError());
	return false;
#else
	pthread_mutex_lock(&_priv->mutex);
	if (_priv->count == 0) {
		pthread_mutex_unlock(&_priv->mutex);
		return false;
	}

	while (_priv->count <= 0) {
		int err = pthread_cond_wait(&_priv->cond, &_priv->mutex);
		CE_ASSERT(err == 0, "pthread_cond_wait: errno = %d", err);
		CE_UNUSED(err);
	}
	_priv->count--;
	pthread_mutex_unlock(&_priv->mutex);
	return true;
#endif // if CROWN_PLATFORM_WINDOWS
}

} // namespace crown
