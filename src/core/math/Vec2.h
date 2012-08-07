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
#include "Types.h"
#include "MathUtils.h"
#include "Str.h"

namespace Crown
{

//!< 2D column vector
class Vec2
{

public:

	real				x, y;

						Vec2();									//!< Constructor, does nothing for efficiency
						Vec2(real val);							//!< Initializes all the components to val
						Vec2(real nx, real ny);					//!< Constructs from two components
						Vec2(const real v[2]);					//!< Constructs from array
						Vec2(const Vec2& a);					//!< Copy constructor
						~Vec2();								//!< Destructor

	real				operator[](uint i) const;				//!< Random access by index
	real&				operator[](uint i);						//!< Random access by index

	Vec2				operator+(const Vec2& a) const;			//!< Addition
	Vec2&				operator+=(const Vec2& a);				//!< Addition
	Vec2 				operator-(const Vec2& a) const;			//!< Subtraction
	Vec2&				operator-=(const Vec2& a);				//!< Subtraction
	Vec2				operator*(real k) const;				//!< Multiplication by scalar
	Vec2&				operator*=(real k);						//!< Multiplication by scalar
	Vec2				operator/(real k) const;				//!< Division by scalar
	Vec2&				operator/=(real k);						//!< Division by scalar
	real				Dot(const Vec2& a) const;				//!< Dot product

	friend Vec2			operator*(real k, const Vec2& a);		//!< For simmetry

	bool				operator==(const Vec2& other) const;	//!< Equality operator
	bool				operator!=(const Vec2& other) const;	//!< Disequality operator
	bool				operator<(const Vec2& other) const;		//!< Returns whether all the components of this vector are smaller than all of the "other" vector
	bool				operator>(const Vec2& other) const;		//!< Returns whether all the components of this vector are greater than all of the "other" vector

	real				GetLength() const;						//!< Returns the vector's length
	real				GetSquaredLength() const;				//!< Returns the vector's squared length
	void				SetLength(real len);					//!< Sets the vector's length
	real				GetAngle() const;
	real				GetAngle2d() const;
	Vec2&				Normalize();							//!< Normalizes the vector
	Vec2				GetNormalized() const;					//!< Returns the normalized vector
	Vec2&				Negate();								//!< Negates the vector (i.e. builds the inverse)
	Vec2				operator-() const;						//!< Negates the vector (i.e. builds the inverse)

	real				GetDistanceTo(const Vec2& a) const;		//!< Returns the distance
	real				GetAngleBetween(const Vec2& a) const;	//!< Returns the angle in radians

	void				Zero();									//!< Builds the zero vector

	real*				ToFloatPtr();							//!< Returns the pointer to the vector's data
	const real*			ToFloatPtr() const;						//!< Returns the pointer to the vector's data
	Str					ToStr() const;							//!< Returns a Str containing the vector's components

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
inline Vec2::~Vec2()
{
}

//-----------------------------------------------------------------------------
inline real Vec2::operator[](uint i) const
{
	assert(i < 2);

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline real& Vec2::operator[](uint i)
{
	assert(i < 2);

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
	assert(k != (real)0.0);

	real inv = (real)(1.0 / k);

	return Vec2(x * inv, y * inv);
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::operator/=(real k)
{
	assert(k != (real)0.0);

	real inv = (real)(1.0 / k);

	x *= inv;
	y *= inv;

	return *this;
}

//-----------------------------------------------------------------------------
inline real Vec2::Dot(const Vec2& a) const
{
	return x * a.x + y * a.y;
}

//-----------------------------------------------------------------------------
inline bool Vec2::operator==(const Vec2& other) const
{
	return Math::Equals(x, other.x) && Math::Equals(y, other.y);
}

//-----------------------------------------------------------------------------
inline bool Vec2::operator!=(const Vec2& other) const
{
	return !Math::Equals(x, other.x) || !Math::Equals(y, other.y);
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
inline real Vec2::GetLength() const
{
	return Math::Sqrt(x * x + y * y);
}

//-----------------------------------------------------------------------------
inline real Vec2::GetSquaredLength() const
{
	return x * x + y * y;
}

//-----------------------------------------------------------------------------
inline void Vec2::SetLength(real len)
{
	Normalize();

	x *= len;
	y *= len;
}

//-----------------------------------------------------------------------------
inline real Vec2::GetAngle() const
{
	return Math::Atan2(y, x);
}

//-----------------------------------------------------------------------------
inline real Vec2::GetAngle2d() const
{
	return Math::Atan2(-y, x);
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::Normalize()
{
	real length = GetLength();

	if (Math::Equals(length, (real)0.0))
	{
		return *this;
	}

	x /= length;
	y /= length;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec2 Vec2::GetNormalized() const
{
	Vec2 tmp(x, y);

	return tmp.Normalize();
}

//-----------------------------------------------------------------------------
inline Vec2& Vec2::Negate()
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
inline real Vec2::GetDistanceTo(const Vec2& a) const
{
	return (*this - a).GetLength();
}

//-----------------------------------------------------------------------------
inline real Vec2::GetAngleBetween(const Vec2& a) const
{
	return Math::Acos(this->Dot(a) / (this->GetLength() * a.GetLength()));
}

//-----------------------------------------------------------------------------
inline void Vec2::Zero()
{
	x = 0.0;
	y = 0.0;
}

//-----------------------------------------------------------------------------
inline real* Vec2::ToFloatPtr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const real* Vec2::ToFloatPtr() const
{
	return &x;
}

//-----------------------------------------------------------------------------
inline Str Vec2::ToStr() const
{
	Str tmp;

	tmp = Str("[ x: ") + x + Str(" y: ") + y + Str(" ]\n");

	return tmp;
}

//-----------------------------------------------------------------------------
inline Vec2 GetProjectedParallel(const Vec2& v, const Vec2& n)
{
	real n_len_q;
	n_len_q = n.GetLength();
	n_len_q = n_len_q * n_len_q;

	return n * (v.Dot(n) / n_len_q);
}

//-----------------------------------------------------------------------------
inline Vec2 GetProjectedPerpendicular(const Vec2& v, const Vec2& n)
{
	return v - GetProjectedParallel(v, n);
}

//-----------------------------------------------------------------------------
inline Vec2 operator*(real k, const Vec2& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
inline bool DeserializeFromStr(Vec2& out, Str& input)
{
	List<Str> coords;
	input.Split(',', coords);

	if (coords.GetSize() != 2)
		return false;

	if (!coords[0].ParseFloat(&out.x))
		return false;
	if (!coords[1].ParseFloat(&out.y))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
inline Str SerializeToStr(const Vec2& in)
{
	return Str(in.x) + ", " + Str(in.y);
}

} // namespace Crown

