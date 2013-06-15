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

/// Interpolation utilities.
namespace interpolation
{

/// Returns the linear interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
static T	linear(const T& p0, const T& p1, float t);

/// Returns the cosine interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
static T	cosine(const T& p0, const T& p1, float t);

/// Returns the cubic interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
static T	cubic(const T& p0, const T& p1, float t);

/// Bezier interpolation
template <typename T>
static T	bezier(const T& p1, const T& p2, const T& p3, const T& p4, float t);

/// Catmull-Rom interpolation
template <typename T>
static T	catmull_rom(const T& p0, const T& p1, const T& p2, const T& p3, float t);

//-----------------------------------------------------------------------------
template <typename T>
inline T linear(const T& p0, const T& p1, float t)
{
	return p0 + (t * (p1 - p0));
}

//-----------------------------------------------------------------------------
template <typename T>
inline T cosine(const T& p0, const T& p1, float t)
{
	float f = t * math::PI;
	float g = (1.0 - math::cos(f)) * 0.5;

	return p0 + (g * (p1 - p0));
}

//-----------------------------------------------------------------------------
template <typename T>
inline T cubic(const T& p0, const T& p1, float t)
{
	float tt = t * t;
	float ttt = tt * t;

	return p0 * (2.0 * ttt - 3.0 * tt + 1.0) + p1 * (3.0 * tt  - 2.0 * ttt);
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

} // namespace interpolation
} // namespace crown

