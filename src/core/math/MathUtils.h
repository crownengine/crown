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

namespace crown
{

/// Math utilities.
namespace math
{

// Constants
const real		PI							= (real)3.1415926535897932;
const real		TWO_PI						= PI * (real)2.0;
const real		HALF_PI						= PI * (real)0.5;
const real		ONEFOURTH_PI				= PI * (real)0.25;

const real		DEG_TO_RAD					= PI / (real)180.0;
const real		RAD_TO_DEG					= (real)1.0 / DEG_TO_RAD;

const real		FOUR_OVER_THREE				= (real)(4.0 / 3.0);
const real		FOUR_OVER_THREE_TIMES_PI	= FOUR_OVER_THREE * PI;

const real		ONE_OVER_THREE				= (real)(1.0 / 3.0);
const real		ONE_OVER_255				= (real)(1.0 / 255.0);

const float		FLOAT_PRECISION				= (real)1.0e-7f;
const double	DOUBLE_PRECISION			= (real)1.0e-9;

bool			equals(float a, float b, float precision = FLOAT_PRECISION);
bool			equals(double a, double b, double precision = DOUBLE_PRECISION);

bool			test_bitmask(int32_t value, int32_t bitmask);	//!< Tests agains a specified bitmask and returns true only if all bits are satisfied
int32_t			set_bitmask(int32_t value, int32_t bitmask);	//!< Sets the specified bitmask
int32_t			unset_bitmask(int32_t value, int32_t bitmask);	//!< Removes the specified bitmask


template <typename T> T		min(const T& a, const T& b);	//!< Returns minimum between two values
template <typename T> T		max(const T& a, const T& b);	//!< Returns maximum between two values
template <typename T> T		avg(const T& a, const T& b);	//!< Returns the arithmetic mean of a and b
template <typename T> T		clamp_to_range(const T& min, const T& max, const T& value);	//!< Clamps a value to a specific range (min < max)
template <typename T> void	swap(T& a, T& b);				//!< Swaps @a and @b

real			deg_to_rad(real deg);			//!< Returns "deg" in radians
real			rad_to_deg(real rad);			//!< Returns "rad" in degrees
uint32_t		next_pow_2(uint32_t x);			//!< Returns the nearest power of two to @x
bool			is_pow_2(uint32_t x);			//!< Returns whether @x is power of two
real			ceil(real x);					//!< Returns the smallest int32_tegral value that is not less than x
real			floor(real x);					//!< Returns the largest int32_tegral value that is not greater than x
real			sqrt(real x);					//!< Returns the square root of @x
real			inv_sqrt(real x);				//!< Returns the inverse square root of @x
real			sin(real x);					//!< Returns the sine of @x
real			cos(real x);					//!< Returns the cosine of @x
real			asin(real x);					//!< Returns the arc sine of @x
real			acos(real x);					//!< Returns the arc cosine of @x
real			tan(real x);					//!< Returns the tangent of @x
real			atan2(real y, real x);			//!< Returns the arc tangent of @y/@x
real			abs(real x);					//!< Returns the absolute value of @x
real			fmod(real n, real d);			//!< Returns the realing-point remainder of numerator/denominator


				//! Returns true if there are solutions and puts them in 'x1' and 'x2' (x1 <= x2)
bool			solve_quadratic_equation(real a, real b, real c, real& x1, real& x2);

//-----------------------------------------------------------------------------
inline bool equals(float a, float b, float precision)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

//-----------------------------------------------------------------------------
inline bool equals(double a, double b, double precision)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

//-----------------------------------------------------------------------------
inline bool test_bitmask(int32_t value, int32_t bitmask)
{
	return (value & bitmask) == bitmask;
}

//-----------------------------------------------------------------------------
inline int32_t set_bitmask(int32_t value, int32_t bitmask)
{
	return value | bitmask;
}

//-----------------------------------------------------------------------------
inline int32_t unset_bitmask(int32_t value, int32_t bitmask)
{
	return value & (~bitmask);
}

//-----------------------------------------------------------------------------
template <typename T>
inline T min(const T& a, const T& b)
{
	return a < b ? a : b;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T max(const T& a, const T& b)
{
	return a < b ? b : a;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T avg(const T& a, const T& b)
{
	return (a + b) * 0.5;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T clamp_to_range(const T& min, const T& max, const T& value)
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
inline void swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

//-----------------------------------------------------------------------------
inline real deg_to_rad(real deg)
{
	return deg * DEG_TO_RAD;
}

//-----------------------------------------------------------------------------
inline real rad_to_deg(real rad)
{
	return rad * RAD_TO_DEG;
}

//-----------------------------------------------------------------------------
inline uint32_t next_pow_2(uint32_t x)
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
inline bool is_pow_2(uint32_t x)
{
	return !(x & (x - 1)) && x;
}

//-----------------------------------------------------------------------------
inline real ceil(real x)
{
	return ceilf(x);
}

//-----------------------------------------------------------------------------
inline real floor(real x)
{
	return floorf(x);
}

//-----------------------------------------------------------------------------
inline real sqrt(real x)
{
	return sqrtf(x);
}

//-----------------------------------------------------------------------------
inline real inv_sqrt(real x)
{
	return 1.0 / sqrt(x);
}

//-----------------------------------------------------------------------------
inline real sin(real x)
{
	return sinf(x);
}

//-----------------------------------------------------------------------------
inline real cos(real x)
{
	return cosf(x);
}

//-----------------------------------------------------------------------------
inline real asin(real x)
{
	return asinf(x);
}

//-----------------------------------------------------------------------------
inline real acos(real x)
{
	return acosf(x);
}

//-----------------------------------------------------------------------------
inline real tan(real x)
{
	return tanf(x);
}

//-----------------------------------------------------------------------------
inline real atan2(real y, real x)
{
	return atan2f(y, x);
}

//-----------------------------------------------------------------------------
inline real abs(real x)
{
	return fabs(x);
}

//-----------------------------------------------------------------------------
inline real fmod(real n, real d)
{
	return ::fmod(n, d);
}

//-----------------------------------------------------------------------------
inline bool solve_quadratic_equation(real a, real b, real c, real& x1, real& x2)
{
	real delta = (b * b) - (4.0 * a * c);

	// If the equation has no real solutions
	if (delta < 0.0)
	{
		return false;
	}

	x1 = (-b + sqrt(delta)) / (2.0 * a);
	x2 = (-b - sqrt(delta)) / (2.0 * a);

	if (x2 > x1)
	{
		swap(x1, x2);
	}

	return true;
}

} // namespace math
} // namespace crown

