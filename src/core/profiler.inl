/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/profiler.h"

namespace crown
{
/// Automatically enters a profile scope when created and leaves it when destroyed.
///
/// @ingroup Device
struct ScopedProfileScope
{
	/// Enters the profile scope @a name.
	explicit ScopedProfileScope(const char *name)
	{
		ENTER_PROFILE_SCOPE(name);
	}

	/// Leaves the profile scope passed to ScopedProfileScope::ScopedProfileScope().
	~ScopedProfileScope()
	{
		LEAVE_PROFILE_SCOPE();
	}

	///
	ScopedProfileScope(const ScopedProfileScope &) = delete;

	///
	ScopedProfileScope &operator=(const ScopedProfileScope &) = delete;
};

} // namespace crown
