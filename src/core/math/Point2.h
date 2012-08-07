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
#include "Vec2.h"
#include "Vec3.h"

namespace Crown
{

/**
	2D point.
*/
class Point2
{

public:

	int						x, y;

							Point2();							//!< Constructor, does nothing for efficiency
							Point2(int nx, int ny);				//! Constructs from two components
							Point2(const int a[2]);				//! Constructs from the array
							Point2(const Point2& a);			//! Copy constructor
							~Point2();							//! Destructor

	int						operator[](uint i) const;			//! Random access by index
	int&					operator[](uint i);					//! Random access by index

	Point2					operator+(const Point2& a) const;	//! Addition
	Point2&					operator+=(const Point2& a);		//! Addition
	Point2					operator-(const Point2& a) const;	//! Subtraction
	Point2&					operator-=(const Point2& a);		//! Subtraction
	Point2					operator*(int k) const;				//! Multiplication by scalar
	Point2&					operator*=(int k);					//! Multiplication by scalar
	Point2					operator/(int k) const;				//! Division by scalar
	Point2&					operator/=(int k);					//! Division by scalar
	int						Dot(const Point2& a);				//! Dot product

	friend Point2			operator*(int k, const Point2& a);	//! For simmetry

	bool					operator==(const Point2& other) const;	//! Equality operator
	bool					operator!=(const Point2& other) const;	//! Disequality operator
	bool					operator<(const Point2& other) const;	//! Returns whether all the components of this point are smaller than all of the "other" point
	bool					operator>(const Point2& other) const;	//! Returns whether all the components of this point are greater than all of the "other" point

	real					GetLength() const;					//! Returns the point's length
	int						GetSquaredLength() const;			//! Returns the point's squared length
	void					Negate();							//! Negates the point (i.e. builds the inverse)

	real					GetDistanceTo(const Point2& a);		//!< Returns the distance
	real					GetAngleBetween(const Point2& a);	//!< Returns the angle in radians

	Point2					operator-() const;					//! Negates the point (i.e. builds the inverse)

	void					Zero();								//! Builds the zero point

	int*					ToIntPtr();	//! Returns the pointer to the point's data
	const int*				ToIntPtr() const;	//! Returns the pointer to the point's data
	Vec2					ToVec2() const;	//! Returns a vector from this point
	Vec3					ToVec3() const;	//! Returns a vector from this point
	Str						ToStr() const;	//! Returns a Str containing the point's components

	static const Point2		ZERO;
	static const Point2		ONE;
	static const Point2		XAXIS;
	static const Point2		YAXIS;
};

//-----------------------------------------------------------------------------
inline Point2::Point2() : x(0), y(0)
{
}

//-----------------------------------------------------------------------------
inline Point2::Point2(int nx, int ny) : x(nx), y(ny)
{
}

//-----------------------------------------------------------------------------
inline Point2::Point2(const int a[2]) : x(a[0]), y(a[1])
{
}

//-----------------------------------------------------------------------------
inline Point2::Point2(const Point2& a) : x(a.x), y(a.y)
{
}

//-----------------------------------------------------------------------------
inline Point2::~Point2()
{
}

//-----------------------------------------------------------------------------
inline int Point2::operator[](uint i) const
{
	assert(i < 2);

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline int& Point2::operator[](uint i)
{
	assert(i < 2);

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Point2 Point2::operator+(const Point2& a) const
{
	return Point2(x + a.x, y + a.y);
}

//-----------------------------------------------------------------------------
inline Point2& Point2::operator+=(const Point2& a)
{
	x += a.x;
	y += a.y;

	return *this;
}

//-----------------------------------------------------------------------------
inline Point2 Point2::operator-(const Point2& a) const
{
	return Point2(x - a.x, y - a.y);
}

//-----------------------------------------------------------------------------
inline Point2& Point2::operator-=(const Point2& a)
{
	x -= a.x;
	y -= a.y;

	return *this;
}

//-----------------------------------------------------------------------------
inline Point2 Point2::operator*(int k) const
{
	return Point2(x * k, y * k);
}

//-----------------------------------------------------------------------------
inline Point2& Point2::operator*=(int k)
{
	x *= k;
	y *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Point2 Point2::operator/(int k) const
{
	assert(k != 0);

	return Point2(x / k, y / k);
}

//-----------------------------------------------------------------------------
inline Point2& Point2::operator/=(int k)
{
	assert(k != 0);

	x /= k;
	y /= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline int Point2::Dot(const Point2& a)
{
	return x * a.x + y * a.y;
}


//-----------------------------------------------------------------------------
inline Point2 operator*(int k, const Point2& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
inline bool Point2::operator==(const Point2& other) const
{
	return x == other.x && y == other.y;
}

//-----------------------------------------------------------------------------
inline bool Point2::operator!=(const Point2& other) const
{
	return x != other.x || y != other.y;
}

//-----------------------------------------------------------------------------
inline bool Point2::operator<(const Point2& other) const
{
	return ((x < other.x) && (y < other.y));
}

//-----------------------------------------------------------------------------
inline bool Point2::operator>(const Point2& other) const
{
	return ((x > other.x) && (y > other.y));
}

//-----------------------------------------------------------------------------
inline real Point2::GetLength() const
{
	return Math::Sqrt((real)(x * x + y * y));
}

//-----------------------------------------------------------------------------
inline int Point2::GetSquaredLength() const
{
	return x * x + y * y;
}

//-----------------------------------------------------------------------------
inline void Point2::Negate()
{
	x = -x;
	y = -y;
}

//-----------------------------------------------------------------------------
inline real Point2::GetDistanceTo(const Point2& a)
{
	return (*this - a).GetLength();
}

//-----------------------------------------------------------------------------
inline real Point2::GetAngleBetween(const Point2& a)
{
	return Math::Acos(this->Dot(a) / (this->GetLength() * a.GetLength()));
}

//-----------------------------------------------------------------------------
inline Point2 Point2::operator-() const
{
	return Point2(-x, -y);
}

//-----------------------------------------------------------------------------
inline void Point2::Zero()
{
	x = y = 0;
}

//-----------------------------------------------------------------------------
inline int* Point2::ToIntPtr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const int* Point2::ToIntPtr() const
{
	return &x;
}

//-----------------------------------------------------------------------------
inline Vec2 Point2::ToVec2() const
{
	return Vec2((real)x, (real)y);
}

//-----------------------------------------------------------------------------
inline Vec3 Point2::ToVec3() const
{
	return Vec3((real)x, (real)y, 0.0);
}

//-----------------------------------------------------------------------------
inline Str Point2::ToStr() const
{
	Str tmp;

	tmp = Str("[ x: ") + x + Str(" y: ") + y + Str(" ]\n");

	return tmp;
}

//-----------------------------------------------------------------------------
inline bool DeserializeFromStr(Point2& out, Str& input)
{
	List<Str> coords;
	input.Split(',', coords);

	if (coords.GetSize() != 2)
		return false;

	if (!coords[0].ParseInt(&out.x))
		return false;
	if (!coords[1].ParseInt(&out.y))
		return false;

	return true;
}

//-----------------------------------------------------------------------------
inline Str SerializeToStr(const Point2& in)
{
	return Str(in.x) + ", " + Str(in.y);
}

} // namespace Crown

