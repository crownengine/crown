/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/thread/condition_variable.h"

namespace crown
{
ConditionVariable::ConditionVariable()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_init(&_cond, NULL);
	CE_ASSERT(err == 0, "pthread_cond_init: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	InitializeConditionVariable(&_cv);
#endif
}

ConditionVariable::~ConditionVariable()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_destroy(&_cond);
	CE_ASSERT(err == 0, "pthread_cond_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	// Do nothing
#endif
}

void ConditionVariable::wait(Mutex& mutex)
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_wait(&_cond, &mutex._mutex);
	CE_ASSERT(err == 0, "pthread_cond_wait: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	SleepConditionVariableCS(&_cv, &mutex._cs, INFINITE);
#endif
}

void ConditionVariable::signal()
{
#if CROWN_PLATFORM_POSIX
	int err = pthread_cond_signal(&_cond);
	CE_ASSERT(err == 0, "pthread_cond_signal: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	WakeConditionVariable(&_cv);
#endif
}

} // namespace crown
