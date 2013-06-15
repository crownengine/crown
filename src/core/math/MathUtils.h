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

#undef min
#undef max

#include "Assert.h"
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
const float		PI							= (float)3.1415926535897932;
const float		TWO_PI						= PI * (float)2.0;
const float		HALF_PI						= PI * (float)0.5;
const float		ONEFOURTH_PI				= PI * (float)0.25;

const float		DEG_TO_RAD					= PI / (float)180.0;
const float		RAD_TO_DEG					= (float)1.0 / DEG_TO_RAD;

const float		FOUR_OVER_THREE				= (float)(4.0 / 3.0);
const float		FOUR_OVER_THREE_TIMES_PI	= FOUR_OVER_THREE * PI;

const float		ONE_OVER_THREE				= (float)(1.0 / 3.0);
const float		ONE_OVER_255				= (float)(1.0 / 255.0);

const float		FLOAT_PRECISION				= (float)1.0e-7f;
const double	DOUBLE_PRECISION			= (float)1.0e-9;

bool			equals(float a, float b, float precision = FLOAT_PRECISION);
bool			equals(double a, double b, double precision = DOUBLE_PRECISION);

/// Tests agains the specified @a bitmask and returns true only if all bits are satisfied
bool			test_bitmask(int32_t value, int32_t bitmask);

/// Sets the specified @a bitmask	
int32_t			set_bitmask(int32_t value, int32_t bitmask);

/// Removes the specified @a bitmask	
int32_t			unset_bitmask(int32_t value, int32_t bitmask);	

/// Returns minimum between @a a and @a b
template <typename T> T		min(const T& a, const T& b);

/// Returns maximum between @a a and @a b
template <typename T> T		max(const T& a, const T& b);

/// Returns the arithmetic mean of @a a and @a b	
template <typename T> T		avg(const T& a, const T& b);

/// Clamps a value to a specific range (min < max)	
template <typename T> T		clamp_to_range(const T& min, const T& max, const T& value);	

/// Swaps @a a and @a b
template <typename T> void	swap(T& a, T& b);				

/// Returns @a deg in radians
float			deg_to_rad(float deg);

/// Returns @a rad in degrees
float			rad_to_deg(float rad);

/// Returns the nearest power of two to @a x
uint32_t		next_pow_2(uint32_t x);

/// Returns whether @a x is a power of two			
bool			is_pow_2(uint32_t x);	

/// Returns the smallest integral value that is not less than @a x
float			ceil(float x);		

/// Returns the largest integral value that is not greater than @a x			
float			floor(float x);	

/// Returns the square root of @a x				
float			sqrt(float x);	

/// Returns the inverse square root of @a x				
float			inv_sqrt(float x);

/// Returns the sine of @a x				
float			sin(float x);	

/// Returns the cosine of @a x				
float			cos(float x);

/// Returns the arc sine of @a x					
float			asin(float x);	

/// Returns the arc cosine of @a x				
float			acos(float x);	

/// Returns the tangent of @a x				
float			tan(float x);		

/// Returns the arc tangent of @a y/@a x			
float			atan2(float y, float x);	

/// Returns the absolute value of @a x		
float			abs(float x);			

/// Returns the floating-point remainder of numerator/denominator		
float			fmod(float n, float d);			

/// Returns true if there are solutions and puts them in 'x1' and 'x2' (x1 <= x2)
bool			solve_quadratic_equation(float a, float b, float c, float& x1, float& x2);

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
	CE_ASSERT(min < max, "Min must be < max");

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
inline float deg_to_rad(float deg)
{
	return deg * DEG_TO_RAD;
}

//-----------------------------------------------------------------------------
inline float rad_to_deg(float rad)
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
inline float ceil(float x)
{
	return ceilf(x);
}

//-----------------------------------------------------------------------------
inline float floor(float x)
{
	return floorf(x);
}

//-----------------------------------------------------------------------------
inline float sqrt(float x)
{
	return sqrtf(x);
}

//-----------------------------------------------------------------------------
inline float inv_sqrt(float x)
{
	return 1.0 / sqrt(x);
}

//-----------------------------------------------------------------------------
inline float sin(float x)
{
	return sinf(x);
}

//-----------------------------------------------------------------------------
inline float cos(float x)
{
	return cosf(x);
}

//-----------------------------------------------------------------------------
inline float asin(float x)
{
	return asinf(x);
}

//-----------------------------------------------------------------------------
inline float acos(float x)
{
	return acosf(x);
}

//-----------------------------------------------------------------------------
inline float tan(float x)
{
	return tanf(x);
}

//-----------------------------------------------------------------------------
inline float atan2(float y, float x)
{
	return atan2f(y, x);
}

//-----------------------------------------------------------------------------
inline float abs(float x)
{
	return fabs(x);
}

//-----------------------------------------------------------------------------
inline float fmod(float n, float d)
{
	return ::fmod(n, d);
}

//-----------------------------------------------------------------------------
inline bool solve_quadratic_equation(float a, float b, float c, float& x1, float& x2)
{
	float delta = (b * b) - (4.0 * a * c);

	// If the equation has no float solutions
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

