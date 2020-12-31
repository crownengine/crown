/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/platform.h"
#include "core/thread/condition_variable.h"
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
	pthread_cond_t cond;
#elif CROWN_PLATFORM_WINDOWS
	CONDITION_VARIABLE cv;
#endif
};

ConditionVariable::ConditionVariable()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(*_priv));
	_priv = new (_data) Private();

#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_init(&_priv->cond, NULL);
	CE_ASSERT(err == 0, "pthread_cond_init: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	InitializeConditionVariable(&_priv->cv);
#endif
}

ConditionVariable::~ConditionVariable()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_destroy(&_priv->cond);
	CE_ASSERT(err == 0, "pthread_cond_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	// Do nothing
#endif
	_priv->~Private();
}

void ConditionVariable::wait(Mutex& mutex)
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_wait(&_priv->cond, (pthread_mutex_t*)mutex.native_handle());
	CE_ASSERT(err == 0, "pthread_cond_wait: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	SleepConditionVariableCS(&_priv->cv, (CRITICAL_SECTION*)mutex.native_handle(), INFINITE);
#endif
}

void ConditionVariable::signal()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_signal(&_priv->cond);
	CE_ASSERT(err == 0, "pthread_cond_signal: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	WakeConditionVariable(&_priv->cv);
#endif
}

} // namespace crown
