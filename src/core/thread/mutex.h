/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

namespace crown
{
/// Mutex.
///
/// @ingroup Thread
struct Mutex
{
	CE_ALIGN_DECL(16, u8 _data[64]);

	///
	Mutex();

	///
	~Mutex();

	///
	Mutex(const Mutex&) = delete;

	///
	Mutex& operator=(const Mutex&) = delete;

	/// Locks the mutex.
	void lock();

	/// Unlocks the mutex.
	void unlock();

	/// Returns the native mutex handle.
	void* native_handle();
};

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
