/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

#define TIME_FMT "%.3fs"

namespace crown
{
namespace time
{
	/// Returns the current time in ticks.
	s64 now();

	/// Returns the ticks in seconds.
	f64 seconds(s64 ticks);

} // namespace time

} // namespace crown
