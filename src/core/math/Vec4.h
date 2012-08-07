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

//!< 4D column vector
class Vec4
{

public:

	real				x, y, z, w;

						Vec4();										//!< Constructor, does nothing for efficiency
						Vec4(real val);								//!< Initializes all the components to val
						Vec4(real nx, real ny, real nz, real nw);	//!< Constructs from four components
						Vec4(const real v[4]);						//!< Constructs from array
						Vec4(const Vec4& a);						//!< Copy constructor
						~Vec4();									//!< Destructor

	real				operator[](uint i) const;					//!< Random access by index
	real&				operator[](uint i);							//!< Random access by index

	Vec4				operator+(const Vec4& a) const;				//!< Addition
	Vec4&				operator+=(const Vec4& a);					//!< Addition
	Vec4 				operator-(const Vec4& a) const;				//!< Subtraction
	Vec4&				operator-=(const Vec4& a);					//!< Subtraction
	Vec4				operator*(real k) const;					//!< Multiplication by scalar
	Vec4&				operator*=(real k);							//!< Multiplication by scalar
	Vec4				operator/(real k) const;					//!< Division by scalar
	Vec4&				operator/=(real k);							//!< Division by scalar
	real				Dot(const Vec4& a) const;					//!< Dot product

	friend Vec4			operator*(real k, const Vec4& a);			//!< For simmetry

	bool				operator==(const Vec4& other) const;		//!< Equality operator
	bool				operator!=(const Vec4& other) const;		//!< Disequality operator
	bool				operator<(const Vec4& other) const;			//!< Returns whether all the components of this vector are smaller than all of the "other" vector
	bool				operator>(const Vec4& other) const;			//!< Returns whether all the components of this vector are greater than all of the "other" vector

	real				GetLength() const;							//!< Returns the vector's length
	real				GetSquaredLength() const;					//!< Returns the vector's squared length
	void				SetLength(real len);						//!< Sets the vector's length
	Vec4&				Normalize();								//!< Normalizes the vector
	Vec4				GetNormalized() const;						//!< Returns the normalized vector
	Vec4&				Negate();									//!< Negates the vector (i.e. builds the inverse)
	Vec4				operator-() const;							//!< Negates the vector (i.e. builds the inverse)

	real				GetDistanceTo(const Vec4& a) const;			//!< Returns the distance
	real				GetAngleBetween(const Vec4& a) const;		//!< Returns the angle in radians

	void				Zero();										//!< Builds the zero vector

	real*				ToFloatPtr();								//!< Returns the pointer to the vector's data
	const real*			ToFloatPtr() const;							//!< Returns the pointer to the vector's data
	Str					ToStr() const;								//!< Returns a Str containing the vector's components

	static const Vec4	ZERO;
	static const Vec4	ONE;
	static const Vec4	XAXIS;
	static const Vec4	YAXIS;
	static const Vec4	ZAXIS;
	static const Vec4	WAXIS;
};

//-----------------------------------------------------------------------------
inline Vec4::Vec4()
{
}

//-----------------------------------------------------------------------------
inline Vec4::Vec4(real val) : x(val), y(val), z(val), w(val)
{
}

//-----------------------------------------------------------------------------
inline Vec4::Vec4(real nx, real ny, real nz, real nw) : x(nx), y(ny), z(nz), w(nw)
{
}

//-----------------------------------------------------------------------------
inline Vec4::Vec4(const real a[4]) : x(a[0]), y(a[1]), z(a[2]), w(a[3])
{
}

//-----------------------------------------------------------------------------
inline Vec4::Vec4(const Vec4& a) : x(a.x), y(a.y), z(a.z), w(a.w)
{
}

//-----------------------------------------------------------------------------
inline Vec4::~Vec4()
{
}

//-----------------------------------------------------------------------------
inline real Vec4::operator[](uint i) const
{
	assert(i < 4);

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline real& Vec4::operator[](uint i)
{
	assert(i < 4);

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator+(const Vec4& a) const
{
	return Vec4(x + a.x, y + a.y, z + a.z, w + a.w);
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::operator+=(const Vec4& a)
{
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator-(const Vec4& a) const
{
	return Vec4(x - a.x, y - a.y, z - a.z, w - a.w);
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::operator-=(const Vec4& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator*(real k) const
{
	return Vec4(x * k, y * k, z * k, w * k);
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::operator*=(real k)
{
	x *= k;
	y *= k;
	z *= k;
	w *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator/(real k) const
{
	assert(k != (real)0.0);

	real inv = (real)(1.0 / k);

	return Vec4(x * inv, y * inv, z * inv, w * inv);
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::operator/=(real k)
{
	assert(k != (real)0.0);

	real inv = (real)(1.0 / k);

	x *= inv;
	y *= inv;
	z *= inv;
	w *= inv;

	return *this;
}

//-----------------------------------------------------------------------------
inline real Vec4::Dot(const Vec4& a) const
{
	return x * a.x + y * a.y + z * a.z + w * a.w;
}

//-----------------------------------------------------------------------------
inline Vec4 operator*(real k, const Vec4& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
inline bool Vec4::operator==(const Vec4& other) const
{
	return Math::Equals(x, other.x) && Math::Equals(y, other.y) && Math::Equals(z, other.z) && Math::Equals(w, other.w);
}

//-----------------------------------------------------------------------------
inline bool Vec4::operator!=(const Vec4& other) const
{
	return !Math::Equals(x, other.x) || !Math::Equals(y, other.y) || !Math::Equals(z, other.z) || !Math::Equals(w, other.w);
}

//-----------------------------------------------------------------------------
inline bool Vec4::operator<(const Vec4& other) const
{
	return ((x < other.x) && (y < other.y) && (z < other.z) && (w < other.w));
}

//-----------------------------------------------------------------------------
inline bool Vec4::operator>(const Vec4& other) const
{
	return ((x > other.x) && (y > other.y) && (z > other.z) && (w > other.w));
}

//-----------------------------------------------------------------------------
inline real Vec4::GetLength() const
{
	return Math::Sqrt(x * x + y * y + z * z + w * w);
}

//-----------------------------------------------------------------------------
inline real Vec4::GetSquaredLength() const
{
	return x * x + y * y + z * z + w * w;
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::Normalize()
{
	real length = GetLength();

	if (Math::Equals(length, (real)0.0))
	{
		return *this;
	}

	length = (real)(1.0 / length);

	x *= length;
	y *= length;
	z *= length;
	w *= length;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::GetNormalized() const
{
	Vec4 tmp(x, y, z, w);

	return tmp.Normalize();
}

//-----------------------------------------------------------------------------
inline Vec4& Vec4::Negate()
{
	x = -x;
	y = -y;
	z = -z;
	w = -w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec4 Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}

//-----------------------------------------------------------------------------
inline real Vec4::GetDistanceTo(const Vec4& a) const
{
	return (*this - a).GetLength();
}

//-----------------------------------------------------------------------------
inline real Vec4::GetAngleBetween(const Vec4& a) const
{
	return Math::Acos(this->Dot(a) / (this->GetLength() * a.GetLength()));
}

//-----------------------------------------------------------------------------
inline void Vec4::Zero()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
	w = 0.0;
}

//-----------------------------------------------------------------------------
inline real* Vec4::ToFloatPtr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const real* Vec4::ToFloatPtr() const
{
	return &x;
}

//-----------------------------------------------------------------------------
inline Str Vec4::ToStr() const
{
	Str tmp;

	tmp = Str("[ x: ") + x + Str(" y: ") + y + Str(" z: ") + z + Str(" w: ") + w + Str(" ]\n");

	return tmp;
}

} // namespace Crown

