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
	Random(int32_t seed);

	/// Returns a pseudo-random integer in the range [0, 32767].
	int32_t integer();

	/// Returns a pseudo-random integer in the range [0, max).
	int32_t integer(int32_t max);

	/// Returns a pseudo-random float in the range [0.0, 1.0].
	float unit_float();

private:

	int32_t _seed;
};

inline Random::Random(int32_t seed)
	: _seed(seed)
{
}

inline int32_t Random::integer()
{
	_seed = 214013 * _seed + 13737667;
	return (_seed >> 16) & 0x7fff;
}

inline int32_t Random::integer(int32_t max)
{
	return (max == 0) ? 0 : integer() % max;
}

inline float Random::unit_float()
{
	return integer() / (float)0x7fff;
}

} // namespace crown
