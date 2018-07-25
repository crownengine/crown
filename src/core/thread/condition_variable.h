/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/thread/mutex.h"

namespace crown
{
/// Condition variable.
///
/// @ingroup Thread
struct ConditionVariable
{
#if CROWN_PLATFORM_POSIX
	pthread_cond_t _cond;
#elif CROWN_PLATFORM_WINDOWS
	CONDITION_VARIABLE _cv;
#endif

	///
	ConditionVariable();

	///
	~ConditionVariable();

	///
	ConditionVariable(const ConditionVariable&) = delete;

	///
	ConditionVariable& operator=(const ConditionVariable&) = delete;

	///
	void wait(Mutex& mutex);

	///
	void signal();
};

} // namespace crown
