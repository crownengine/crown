/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

namespace crown
{
/// Atomic integer.
///
/// @ingroup Thread
struct AtomicInt
{
	s32 _val;

	/// Initialization is not atomic.
	AtomicInt(s32 val);

	///
	s32 load();

	///
	void store(s32 val);
};

} // namespace crown
