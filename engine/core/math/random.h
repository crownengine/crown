/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

namespace crown
{

///	Pseudo-random number generator.
///
///	Uses LCG algorithm: fast and compatible with the standard C rand().
struct Random
{
public:

	/// Initializes the generator with the given @a seed.
				Random(int32_t seed);

	/// Returns a pseudo-random integer in the range [0, 32767].
	int32_t		integer();

	/// Returns a pseudo-random integer in the range [0, max).
	int32_t		integer(int32_t max);

	/// Returns a pseudo-random float in the range [0.0, 1.0].
	float		unit_float();

private:

	int32_t		m_seed;
};

//-----------------------------------------------------------------------------
inline Random::Random(int32_t seed) : m_seed(seed)
{
}

//-----------------------------------------------------------------------------
inline int32_t Random::integer()
{
	m_seed = 214013 * m_seed + 13737667;

	return (m_seed >> 16) & 0x7FFF;
}

//-----------------------------------------------------------------------------
inline int32_t Random::integer(int32_t max)
{
	return (max == 0) ? 0 : integer() % max;
}

//-----------------------------------------------------------------------------
inline float Random::unit_float()
{
	return integer() / (float) 0x7FFF;
}

} // namespace crown

