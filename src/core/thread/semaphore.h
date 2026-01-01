/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

namespace crown
{
/// Semaphore.
///
/// @ingroup Thread.
struct Semaphore
{
	struct Private *_priv;
	CE_ALIGN_DECL(16, u8 _data[128]);

	///
	Semaphore();

	///
	~Semaphore();

	///
	Semaphore(const Semaphore &) = delete;

	///
	Semaphore &operator=(const Semaphore &) = delete;

	///
	void post(u32 count = 1);

	///
	void wait();

	///
	bool try_wait();
};

} // namespace crown
