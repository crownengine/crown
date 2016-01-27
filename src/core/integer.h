/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{
inline u32 next_pow_2(u32 x)
{
	x--;

	x = (x >> 1) | x;
	x = (x >> 2) | x;
	x = (x >> 4) | x;
	x = (x >> 8) | x;
	x = (x >> 16) | x;

	return ++x;
}

inline bool is_pow_2(u32 x)
{
	return !(x & (x - 1)) && x;
}

} // namespace crown
