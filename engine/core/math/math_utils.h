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

#include <math.h>

#include "assert.h"
#include "types.h"
#include "memory.h"

namespace crown
{

/// Math utilities.
///
/// @ingroup Math
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
const float		ONE_OVER_FOUR				= (float)(1.0 / 4.0);
const float		ONE_OVER_255				= (float)(1.0 / 255.0);

const float		FLOAT_PRECISION				= (float)1.0e-7f;
const double	DOUBLE_PRECISION			= (float)1.0e-9;

//-----------------------------------------------------------------------------
inline bool equals(float a, float b, float precision  = FLOAT_PRECISION)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

//-----------------------------------------------------------------------------
inline bool equals(double a, double b, double precision  = DOUBLE_PRECISION)
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
	return (a + b + c) * ONE_OVER_THREE;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T avg(const T& a, const T& b, const T& c, const T& d)
{
	return (a + b + c + d) * ONE_OVER_FOUR;
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

/// Returns the base64-encoded @a data.
inline char* base64_encode(const unsigned char* data, size_t in_len, size_t* out_len)
{
	const char encoding_table[] =
	{
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};

	const int mod_table[] = { 0, 2, 1 };

	*out_len = 4 * ((in_len + 2) / 3);

	char *encoded_data = (char*) default_allocator().allocate(*out_len);
	if (encoded_data == NULL) return NULL;

	for (size_t i = 0, j = 0; i < in_len;)
	{
		uint32_t octet_a = i < in_len ? (unsigned char)data[i++] : 0;
		uint32_t octet_b = i < in_len ? (unsigned char)data[i++] : 0;
		uint32_t octet_c = i < in_len ? (unsigned char)data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[in_len % 3]; i++)
	{
		encoded_data[*out_len - 1 - i] = '=';
	}

	(*out_len) += 1;
	encoded_data[(*out_len) -1] = '\0';

	return encoded_data;
}

// Decodes the base64-encoded @a data.
inline unsigned char* base64_decode(const char *data, size_t in_len, size_t* out_len)
{
	CE_ASSERT(in_len % 4 == 0, "Input length not a multiple of 4");

	const unsigned char decoding_table[256] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F,
		0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
		0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
		0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
		0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
		0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
		0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
		0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
		0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	*out_len = in_len / 4 * 3;
	if (data[in_len - 1] == '=') (*out_len)--;
	if (data[in_len - 2] == '=') (*out_len)--;

	unsigned char *decoded_data = (unsigned char*) default_allocator().allocate(*out_len);
	if (decoded_data == NULL) return NULL;

	for (size_t i = 0, j = 0; i < in_len;) {

		uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char) data[i++]];
		uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char) data[i++]];
		uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char) data[i++]];
		uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char) data[i++]];

		uint32_t triple = (sextet_a << 3 * 6)
		+ (sextet_b << 2 * 6)
		+ (sextet_c << 1 * 6)
		+ (sextet_d << 0 * 6);

		if (j < *out_len) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < *out_len) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < *out_len) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
	}

	return decoded_data;
}

} // namespace math
} // namespace crown
