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

#include <cassert>
#include <cmath>
#include "Types.h"
#include <cstdio>

#define BIT(i) (1 << i)

namespace Crown
{

/**
	Math utilities.
*/
class Math
{

public:

							/**
								Returns whether "a" and "b" are equal according to the specified precision.
							@warning:
								Use this only for comparing very little values. 0.0 to 10.0 should
								be fine.
							*/
	static bool				Equals(float a, float b, float precision = Math::FLOAT_PRECISION);
							/**
								Returns whether "a" and "b" are equal according to the specified precision.
							@warning:
								Use this only for comparing very little values. 0.0 to 10.0 should
								be fine.
							*/
	static bool				Equals(double a, double b, double precision = Math::DOUBLE_PRECISION);

	static bool				TestBitmask(int value, int bitmask);	//!< Tests agains a specified bitmask and returns true only if all bits are satisfied
	static int				SetBitmask(int value, int bitmask);		//!< Sets the specified bitmask
	static int				UnsetBitmask(int value, int bitmask);	//!< Removes the specified bitmask

	template <typename T>
	static T				Min(const T& a, const T& b);		//!< Returns minimum between two values
	template <typename T>
	static T				Max(const T& a, const T& b);		//!< Returns maximum between two values
	template <typename T>
	static T				Avg(const T& a, const T& b);		//!< Returns the arithmetic mean of a and b
	template <typename T>
	static T				ClampToRange(const T& min, const T& max, const T& value);	//!< Clamps a value to a specific range (min < max)
	template <typename T>
	static void				Swap(T& a, T& b);					//!< Swaps two values

	static real				DegToRad(real deg);					//!< Returns "deg" in radians
	static real				RadToDeg(real rad);					//!< Returns "rad" in degrees
	static uint				NextPow2(uint x);					//!< Returns the nearest power of two to "x"
	static bool				IsPow2(uint x);						//!< Returns whether "x" is power of two
	static real				Ceil(real x);						//!< Returns the smallest integral value that is not less than x
	static real				Floor(real x);						//!< Returns the largest integral value that is not greater than x
	static real				Sqrt(real x);						//!< Returns the square root of "x"
	static real				InvSqrt(real x);					//!< Returns the inverse square root of "x"
	static real				Sin(real x);						//!< Returns the sine of "x"
	static real				Cos(real x);						//!< Returns the cosine of "x"
	static real				Asin(real x);						//!< Returns the arc sine of "x"
	static real				Acos(real x);						//!< Returns the arc cosine of "x"
	static real				Tan(real x);						//!< Returns the tangent of "x"
	static real				Atan2(real y, real x);				//!< Returns the arc tangent of "y" / "x"
	static real				Abs(real x);						//!< Returns the absolute value of "x"
	static real				FMod(real n, real d);				//!< Returns the realing-point remainder of numerator/denominator


							//! Returns true if there are solutions and puts them in 'x1' and 'x2' (x1 <= x2)
	static bool				SolveQuadraticEquation(real a, real b, real c, real& x1, real& x2);

	// Useful constants
	static const real		PI;
	static const real		TWO_PI;
	static const real		HALF_PI;
	static const real		ONEFOURTH_PI;

	static const real		DEG_TO_RAD;
	static const real		RAD_TO_DEG;

	static const real		FOUR_OVER_THREE;
	static const real		FOUR_OVER_THREE_TIMES_PI;

	static const real		ONE_OVER_THREE;
	static const real		ONE_OVER_255;

	static const float		FLOAT_PRECISION;
	static const double		DOUBLE_PRECISION;

private:

	// Disable construction
	Math();
};

//-----------------------------------------------------------------------------
inline bool Math::Equals(float a, float b, float precision)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

//-----------------------------------------------------------------------------
inline bool Math::Equals(double a, double b, double precision)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

//-----------------------------------------------------------------------------
inline bool Math::TestBitmask(int value, int bitmask)
{
	return (value & bitmask) == bitmask;
}

//-----------------------------------------------------------------------------
inline int Math::SetBitmask(int value, int bitmask)
{
	return value | bitmask;
}

//-----------------------------------------------------------------------------
inline int Math::UnsetBitmask(int value, int bitmask)
{
	return value & (~bitmask);
}

//-----------------------------------------------------------------------------
template <typename T>
inline T Math::Min(const T& a, const T& b)
{
	return a < b ? a : b;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T Math::Max(const T& a, const T& b)
{
	return a < b ? b : a;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T Math::Avg(const T& a, const T& b)
{
	return (a + b) * 0.5;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T Math::ClampToRange(const T& min, const T& max, const T& value)
{
	assert(min < max);

	if (value > max)
	{
		return max;
	}

	if (value < min)
	{
		return min;
	}

	return value;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Math::Swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

//-----------------------------------------------------------------------------
inline real Math::DegToRad(real deg)
{
	return deg * Math::DEG_TO_RAD;
}

//-----------------------------------------------------------------------------
inline real Math::RadToDeg(real rad)
{
	return rad * Math::RAD_TO_DEG;
}

//-----------------------------------------------------------------------------
inline uint Math::NextPow2(uint x)
{
	x--;

	x = (x >> 1) | x;
	x = (x >> 2) | x;
	x = (x >> 4) | x;
	x = (x >> 8) | x;
	x = (x >> 16) | x;

	return ++x;
}

//-----------------------------------------------------------------------------
inline bool Math::IsPow2(uint x)
{
	return !(x & (x - 1)) && x;
}

//-----------------------------------------------------------------------------
inline real Math::Ceil(real x)
{
	return ceilf(x);
}

//-----------------------------------------------------------------------------
inline real Math::Floor(real x)
{
	return floorf(x);
}

//-----------------------------------------------------------------------------
inline real Math::Sqrt(real x)
{
	return sqrtf(x);
}

//-----------------------------------------------------------------------------
inline real Math::InvSqrt(real x)
{
	return 1.0 / Math::Sqrt(x);
}

//-----------------------------------------------------------------------------
inline real Math::Sin(real x)
{
	return sinf(x);
}

//-----------------------------------------------------------------------------
inline real Math::Cos(real x)
{
	return cosf(x);
}

//-----------------------------------------------------------------------------
inline real Math::Asin(real x)
{
	return asinf(x);
}

//-----------------------------------------------------------------------------
inline real Math::Acos(real x)
{
	return acosf(x);
}

//-----------------------------------------------------------------------------
inline real Math::Tan(real x)
{
	return tanf(x);
}

//-----------------------------------------------------------------------------
inline real Math::Atan2(real y, real x)
{
	return atan2f(y, x);
}

//-----------------------------------------------------------------------------
inline real Math::Abs(real x)
{
	return fabs(x);
}

//-----------------------------------------------------------------------------
inline real Math::FMod(real n, real d)
{
	return fmod(n, d);
}

//-----------------------------------------------------------------------------
inline bool Math::SolveQuadraticEquation(real a, real b, real c, real& x1, real& x2)
{
	real delta = (b * b) - (4.0 * a * c);

	// If the equation has no real solutions
	if (delta < 0.0)
	{
		return false;
	}

	x1 = (-b + Math::Sqrt(delta)) / (2.0 * a);
	x2 = (-b - Math::Sqrt(delta)) / (2.0 * a);

	if (x2 > x1)
	{
		Swap(x1, x2);
	}

	return true;
}

} // namespace Crown

