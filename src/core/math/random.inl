/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/math/random.h"
#include "core/time.h"

namespace crown
{
inline Random::Random()
	: _seed((u32)time::now())
{
}

inline Random::Random(s32 seed)
	: _seed((u32)seed)
{
}

inline s32 Random::integer()
{
	_seed = 214013u * _seed + 13737667u;
	return (_seed >> 16) & 0x7fff;
}

inline s32 Random::integer(s32 max)
{
	return (max == 0) ? 0 : integer() % max;
}

inline f32 Random::unit_float()
{
	return integer() / (f32)0x7fff;
}

} // namespace crown
