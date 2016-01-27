/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{
/// Pseudo-random number generator.
///
/// Uses LCG algorithm: fast and compatible with the standard C rand().
struct Random
{
	/// Initializes the generator with the given @a seed.
	Random(s32 seed);

	/// Returns a pseudo-random integer in the range [0, 32767].
	s32 integer();

	/// Returns a pseudo-random integer in the range [0, max).
	s32 integer(s32 max);

	/// Returns a pseudo-random f32 in the range [0.0, 1.0].
	f32 unit_float();

private:

	s32 _seed;
};

inline Random::Random(s32 seed)
	: _seed(seed)
{
}

inline s32 Random::integer()
{
	_seed = 214013 * _seed + 13737667;
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
