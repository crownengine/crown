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

#include "Types.h"
#include "MathUtils.h"
#include "Vec3.h"

namespace Crown
{

/**
	Sphere.

	Used mainly for collision detection and intersection tests.
*/
class Sphere
{

public:

	Vec3			c;
	real			r;

					Sphere();									//!< Constructor
					Sphere(const Vec3& center, real radius);	//!< Constructs from center and radius
					Sphere(const Sphere& a);					//!< Copy constructor
					~Sphere();									//!< Destructor

	const Vec3&		GetCenter() const;							//!< Returns the center
	real			GetRadius() const;							//!< Returns the radius
	real			GetVolume() const;							//!< Returns the volume

	void			SetCenter(const Vec3& center);				//!< Sets the center
	void			SetRadius(real radius);					//!< Sets the radius

	void			AddPoint(const Vec3& p);					//!< Adds a point to the Sphere
	void			AddSphere(const Sphere& s);					//!< Adds a Sphere to the Sphere

	bool			ContainsPoint(const Vec3& p) const;			//!< Returns whether point "p" is contained
};

//-----------------------------------------------------------------------------
inline Sphere::Sphere()
{
}

//-----------------------------------------------------------------------------
inline Sphere::Sphere(const Vec3& center, real radius) : c(center), r(radius)
{
}

//-----------------------------------------------------------------------------
inline Sphere::Sphere(const Sphere& a) : c(a.c), r(a.r)
{
}

//-----------------------------------------------------------------------------
inline Sphere::~Sphere()
{
}

//-----------------------------------------------------------------------------
inline const Vec3& Sphere::GetCenter() const
{
	return c;
}

//-----------------------------------------------------------------------------
inline real Sphere::GetRadius() const
{
	return r;
}

//-----------------------------------------------------------------------------
inline real Sphere::GetVolume() const
{
	return Math::FOUR_OVER_THREE_TIMES_PI * r * r * r;
}

//-----------------------------------------------------------------------------
inline void Sphere::SetCenter(const Vec3& center)
{
	c = center;
}

//-----------------------------------------------------------------------------
inline void Sphere::SetRadius(real radius)
{
	this->r = radius;
}

//-----------------------------------------------------------------------------
inline void Sphere::AddPoint(const Vec3& p)
{
	real dist = (p - c).GetSquaredLength();

	if (dist < r * r)
	{
		return;
	}

	r = Math::Sqrt(dist);
}

//-----------------------------------------------------------------------------
inline void Sphere::AddSphere(const Sphere& s)
{
	real dist = (s.c - c).GetSquaredLength();

	if (dist < (s.r + r) * (s.r + r))
		if (s.r * s.r > r * r)
		{
			r = Math::Sqrt(dist + s.r * s.r);
		}
}

//-----------------------------------------------------------------------------
inline bool Sphere::ContainsPoint(const Vec3& p) const
{
	real dist = (p - c).GetSquaredLength();
	return (dist < r * r);
}

} // namespace Crown

