/*
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

/**
	Pseudo-random number generator.

	Uses LCG algorithm: fast and compatible with the standard C rand().
*/ 
class Random
{
public:

				Random(int32_t seed);	//!< Constructor

	int32_t		get_int32_t();			//!< Returns a pseudo-random int32_teger in the range [0, 32767]
	int32_t		get_int32_t(int32_t max);	//!< Returns a pseudo-random int32_teger in the range [0, max)
	float		get_unit_float();	//!< Returns a pseudo-random float in the range [0.0, 1.0].

private:

	int32_t		mSeed;
};

//-----------------------------------------------------------------------------
inline Random::Random(int32_t seed) : mSeed(seed)
{
}

//-----------------------------------------------------------------------------
inline int32_t Random::get_int32_t()
{
	mSeed = 214013 * mSeed + 13737667;

	return (mSeed >> 16) & 0x7FFF;
}

//-----------------------------------------------------------------------------
inline int32_t Random::get_int32_t(int32_t max)
{
	return (max == 0) ? 0 : get_int32_t() % max;
}

//-----------------------------------------------------------------------------
inline float Random::get_unit_float()
{
	return get_int32_t() / (float) 0x7FFF;
}

} // namespace crown

