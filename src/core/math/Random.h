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

namespace Crown
{

/**
	Pseudo-random number generator.

	Uses LCG algorithm: fast and compatible with the standard C rand().
*/ 
class Random
{

public:

			Random(int seed);	//!< Constructor

	int		GetInt();			//!< Returns a pseudo-random integer in the range [0, 32767]
	int		GetInt(int max);	//!< Returns a pseudo-random integer in the range [0, max)
	float	GetUnitFloat();		//!< Returns a pseudo-random unit float in the range [0.0, 1.0].

private:

	int		mSeed;
};

//-----------------------------------------------------------------------------
inline Random::Random(int seed) : mSeed(seed)
{
}

//-----------------------------------------------------------------------------
inline int Random::GetInt()
{
	mSeed = 214013 * mSeed + 13737667;

	return (mSeed >> 16) & 0x7FFF;
}

//-----------------------------------------------------------------------------
inline int Random::GetInt(int max)
{
	return (max == 0) ? 0 : GetInt() % max;
}

//-----------------------------------------------------------------------------
inline float Random::GetUnitFloat()
{
	return GetInt() / (float) 0x7FFF;
}

} // namespace Crown

