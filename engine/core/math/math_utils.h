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

#include "assert.h"
#include "types.h"
#include <math.h>

namespace crown
{

/// Math utilities.
///
/// @ingroup Math
namespace math
{

// Constants
const float PI = 3.1415926535897932f;
const float TWO_PI = PI * 2.0f;
const float HALF_PI = PI * 0.5f;
const float FLOAT_PRECISION = 1.0e-7f;
const double DOUBLE_PRECISION = 1.0e-9;

//-----------------------------------------------------------------------------
inline bool equals(float a, float b, float precision = FLOAT_PRECISION)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

//-----------------------------------------------------------------------------
inline bool equals(double a, double b, double precision = DOUBLE_PRECISION)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

//-----------------------------------------------------------------------------
template <typename T>
inline T min(const T& a, const T& b)
{
	return a < b ? a : b;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T min(const T& a, const T& b, const T& c)
{
	return math::min(math::min(a, b), math::min(a, c));
}

//-----------------------------------------------------------------------------
template <typename T>
inline T min(const T& a, const T& b, const T& c, const T& d)
{
	return math::min(math::min(a, b, c), math::min(a, b, d), math::min(a, c, d));
}

//-----------------------------------------------------------------------------
template <typename T>
inline T max(const T& a, const T& b)
{
	return a < b ? b : a;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T max(const T& a, const T& b, const T& c)
{
	return math::max(math::max(a, b), math::max(a, c));
}

//-----------------------------------------------------------------------------
template <typename T>
inline T max(const T& a, const T& b, const T& c, const T& d)
{
	return math::max(math::max(a, b, c), math::max(a, b, d), math::max(a, c, d));
}

//-----------------------------------------------------------------------------
template <typename T>
inline T avg(const T& a, const T& b)
{
	return (a + b) * 0.5;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T avg(const T& a, const T& b, const T& c)
{
	return (a + b + c) * float(1.0 / 3.0);
}

//-----------------------------------------------------------------------------
template <typename T>
inline T avg(const T& a, const T& b, const T& c, const T& d)
{
	return (a + b + c + d) * float(1.0 / 4.0);
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
	return deg * float(PI / 180.0);
}

//-----------------------------------------------------------------------------
inline float rad_to_deg(float rad)
{
	return rad * float(180.0 / PI);
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
	return 1.0f / sqrt(x);
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
	float delta = (b * b) - (4.0f * a * c);

	// If the equation has no float solutions
	if (delta < 0.0)
	{
		return false;
	}

	x1 = (-b + sqrt(delta)) / (2.0f * a);
	x2 = (-b - sqrt(delta)) / (2.0f * a);

	if (x2 > x1)
	{
		swap(x1, x2);
	}

	return true;
}

/// Returns the linear interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
inline T linear(const T& p0, const T& p1, float t)
{
	return p0 + (t * (p1 - p0));
}

/// Returns the cosine interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
inline T cosine(const T& p0, const T& p1, float t)
{
	float f = t * math::PI;
	float g = (1.0 - math::cos(f)) * 0.5;

	return p0 + (g * (p1 - p0));
}

/// Returns the cubic interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
inline T cubic(const T& p0, const T& p1, float t)
{
	float tt = t * t;
	float ttt = tt * t;

	return p0 * (2.0 * ttt - 3.0 * tt + 1.0) + p1 * (3.0 * tt  - 2.0 * ttt);
}

/// Bezier interpolation
template <typename T>
inline T bezier(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
	float u = 1.0 - t;
	float tt = t * t ;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	T tmp = (uuu * p0) +
			(3 * uu * t * p1) +
			(3 * u * tt * p2) +
			(ttt * p3);

	return tmp;
}

/// Catmull-Rom interpolation
template <typename T>
inline T catmull_rom(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
	float tt = t * t;
	float ttt = tt * t;

	T tmp = (2.0 * p1) +
			((-p0 + p2) * t) +
			(((2.0 * p0) - (5.0 * p1) + (4.0 * p2) - p3) * tt) +
			((-p0 + (3.0 * p1) + (-3.0 * p2) + p3) * ttt);

	return tmp * 0.5;
}

} // namespace math
} // namespace crown
