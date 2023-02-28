/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/thread/types.h"

namespace crown
{
/// Condition variable.
///
/// @ingroup Thread
struct ConditionVariable
{
	struct Private *_priv;
	CE_ALIGN_DECL(16, u8 _data[64]);

	///
	ConditionVariable();

	///
	~ConditionVariable();

	///
	ConditionVariable(const ConditionVariable &) = delete;

	///
	ConditionVariable &operator=(const ConditionVariable &) = delete;

	///
	void wait(Mutex &mutex);

	///
	void signal();
};

} // namespace crown
