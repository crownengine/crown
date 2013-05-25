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

#include "Assert.h"
#include "Types.h"
#include "MathUtils.h"

namespace crown
{

/// 2D column vector.
class Vec2
{
public:

	real				x, y;

	/// Does nothing for efficiency.
						Vec2();		

	/// Initializes all the components to val							
						Vec2(real val);	

	/// Constructs from two components						
						Vec2(real nx, real ny);

	/// Constructs from array
						Vec2(const real v[2]);
						Vec2(const Vec2& a);					

	/// Random access by index
	real				operator[](uint32_t i) const;

	/// Random access by index			
	real&				operator[](uint32_t i);					

	Vec2				operator+(const Vec2& a) const;			
	Vec2&				operator+=(const Vec2& a);				
	Vec2 				operator-(const Vec2& a) const;			
	Vec2&				operator-=(const Vec2& a);				
	Vec2				operator*(real k) const;				
	Vec2&				operator*=(real k);						
	Vec2				operator/(real k) const;				
	Vec2&				operator/=(real k);

	/// Dot product						
	real				dot(const Vec2& a) const;				

	/// For simmetry
	friend Vec2			operator*(real k, const Vec2& a);		

	bool				operator==(const Vec2& other) const;	
	bool				operator!=(const Vec2& other) const;

	/// Returns whether all the components of this vector are smaller than all of the @other vector	
	bool				operator<(const Vec2& other) const;		

	/// Returns whether all the components of this vector are greater than all of the @other vector
	bool				operator>(const Vec2& other) const;		

	/// Returns the vector's length
	real				length() const;

	/// Returns the vector's squared length							
	real				squared_length() const;

	/// Sets the vector's length					
	void				set_length(real len);					
	real				get_angle() const;
	real				get_angle_2d() const;

	/// Normalizes the vector
	Vec2&				normalize();

	/// Returns the normalized vector							
	Vec2				get_normalized() const;

	/// Negates the vector (i.e. builds the inverse)					
	Vec2&				negate();

	/// Negates the vector (i.e. builds the inverse)								
	Vec2				operator-() const;						

	/// Returns the distance
	real				get_distance_to(const Vec2& a) const;

	/// Returns the angle in radian	
	real				get_angle_between(const Vec2& a) const;

	/// Sets all components to zero
	void				zero();

	/// Returns the pointer to the vector's data
	real*				to_float_ptr();	

	/// Returns the pointer to the vector's data						
	const real*			to_float_ptr() const;					

	static const Vec2	ZERO;
	static const Vec2	ONE;
	static const Vec2	XAXIS;
	static const Vec2	YAXIS;
};

//-----------------------------------------------------------------------------
inline Vec2::Vec2()
{
}

//-----------------------------------------------------------------------------
inline Vec2::Vec2(real val) : x(val), y(val)
{
}

//-----------------------------------------------------------------------------
inline Vec2::Vec2(real nx, real ny) : x(nx), y(ny)
{
}

//-----------------------------------------------------------------------------
inline Vec2::Vec2(const real a[2]) : x(a[0]), y(a[1])
{
}

//-----------------------------------------------------------------------------
inline Vec2::Vec2(const Vec2& a) : x(a.x), y(a.y)
{
}

//-----------------------------------------------------------------------------
inline real Vec2::operator[](uint32_t i) const
{
	ce_assert(i < 2, "Index must be < 2");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline real& Vec2::operator[](uint32_t i)
{
	ce_assert(i < 2, "Index must be < 2");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vec2 Vec2::operator+(const Vec2& a) const
{
	return Vec2(x + a.x, y + a.y);
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::operator+=(const Vec2& a)
{
	x += a.x;
	y += a.y;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec2 Vec2::operator-(const Vec2& a) const
{
	return Vec2(x - a.x, y - a.y);
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::operator-=(const Vec2& a)
{
	x -= a.x;
	y -= a.y;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec2 Vec2::operator*(real k) const
{
	return Vec2(x * k, y * k);
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::operator*=(real k)
{
	x *= k;
	y *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec2 Vec2::operator/(real k) const
{
	ce_assert(k != (real)0.0, "Division by zero");

	real inv = (real)(1.0 / k);

	return Vec2(x * inv, y * inv);
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::operator/=(real k)
{
	ce_assert(k != (real)0.0, "Division by zero");

	real inv = (real)(1.0 / k);

	x *= inv;
	y *= inv;

	return *this;
}

//-----------------------------------------------------------------------------
inline real Vec2::dot(const Vec2& a) const
{
	return x * a.x + y * a.y;
}

//-----------------------------------------------------------------------------
inline bool Vec2::operator==(const Vec2& other) const
{
	return math::equals(x, other.x) && math::equals(y, other.y);
}

//-----------------------------------------------------------------------------
inline bool Vec2::operator!=(const Vec2& other) const
{
	return !math::equals(x, other.x) || !math::equals(y, other.y);
}

//-----------------------------------------------------------------------------
inline bool Vec2::operator<(const Vec2& other) const
{
	return ((x < other.x) && (y < other.y));
}

//-----------------------------------------------------------------------------
inline bool Vec2::operator>(const Vec2& other) const
{
	return ((x > other.x) && (y > other.y));
}

//-----------------------------------------------------------------------------
inline real Vec2::length() const
{
	return math::sqrt(x * x + y * y);
}

//-----------------------------------------------------------------------------
inline real Vec2::squared_length() const
{
	return x * x + y * y;
}

//-----------------------------------------------------------------------------
inline void Vec2::set_length(real len)
{
	normalize();

	x *= len;
	y *= len;
}

//-----------------------------------------------------------------------------
inline real Vec2::get_angle() const
{
	return math::atan2(y, x);
}

//-----------------------------------------------------------------------------
inline real Vec2::get_angle_2d() const
{
	return math::atan2(-y, x);
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::normalize()
{
	real len = length();

	if (math::equals(len, (real)0.0))
	{
		return *this;
	}

	x /= len;
	y /= len;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec2 Vec2::get_normalized() const
{
	Vec2 tmp(x, y);

	return tmp.normalize();
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::negate()
{
	x = -x;
	y = -y;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec2 Vec2::operator-() const
{
	return Vec2(-x, -y);
}

//-----------------------------------------------------------------------------
inline real Vec2::get_distance_to(const Vec2& a) const
{
	return (*this - a).length();
}

//-----------------------------------------------------------------------------
inline real Vec2::get_angle_between(const Vec2& a) const
{
	return math::acos(this->dot(a) / (this->length() * a.length()));
}

//-----------------------------------------------------------------------------
inline void Vec2::zero()
{
	x = 0.0;
	y = 0.0;
}

//-----------------------------------------------------------------------------
inline real* Vec2::to_float_ptr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const real* Vec2::to_float_ptr() const
{
	return &x;
}

//-----------------------------------------------------------------------------
inline Vec2 get_projected_parallel(const Vec2& v, const Vec2& n)
{
	real n_len_q;
	n_len_q = n.length();
	n_len_q = n_len_q * n_len_q;

	return n * (v.dot(n) / n_len_q);
}

//-----------------------------------------------------------------------------
inline Vec2 get_projected_perpendicular(const Vec2& v, const Vec2& n)
{
	return v - get_projected_parallel(v, n);
}

//-----------------------------------------------------------------------------
inline Vec2 operator*(real k, const Vec2& a)
{
	return a * k;
}

} // namespace crown

