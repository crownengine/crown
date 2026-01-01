/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	struct Private *_priv;
	CE_ALIGN_DECL(16, u8 _data[64]);

	///
	Mutex();

	///
	~Mutex();

	///
	Mutex(const Mutex &) = delete;

	///
	Mutex &operator=(const Mutex &) = delete;

	/// Locks the mutex.
	void lock();

	/// Unlocks the mutex.
	void unlock();

	/// Returns the native mutex handle.
	void *native_handle();
};

} // namespace crown
