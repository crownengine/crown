/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/thread/mutex.h"

namespace crown
{
/// Automatically locks a mutex when created and unlocks when destroyed.
///
/// @ingroup Thread
struct ScopedMutex
{
	Mutex& _mutex;

	/// Locks the mutex @a m.
	ScopedMutex(Mutex& m)
		: _mutex(m)
	{
		_mutex.lock();
	}

	/// Unlocks the mutex passed to ScopedMutex::ScopedMutex()
	~ScopedMutex()
	{
		_mutex.unlock();
	}

	///
	ScopedMutex(const ScopedMutex&) = delete;

	///
	ScopedMutex& operator=(const ScopedMutex&) = delete;
};

} // namespace crown
