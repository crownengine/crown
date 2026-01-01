/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/thread/types.h"

namespace crown
{
/// Result of a ConditionVariable::wait().
///
/// @ingroup Thread
struct WaitResult
{
	enum Enum
	{
		SUCCESS,
		TIMEOUT,
		UNKNOWN
	} error;
};

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
	WaitResult wait(Mutex &mutex, u32 ms = 0u);

	///
	void signal();
};

} // namespace crown
