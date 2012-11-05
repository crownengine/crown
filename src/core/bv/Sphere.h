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

namespace crown
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

	const Vec3&		get_center() const;							//!< Returns the center
	real			get_radius() const;							//!< Returns the radius
	real			get_volume() const;							//!< Returns the volume

	void			set_center(const Vec3& center);				//!< Sets the center
	void			set_radius(real radius);					//!< Sets the radius

	void			add_point(const Vec3& p);					//!< Adds a point to the Sphere
	void			add_sphere(const Sphere& s);				//!< Adds a Sphere to the Sphere

	bool			contains_point(const Vec3& p) const;		//!< Returns whether point "p" is contained
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
inline const Vec3& Sphere::get_center() const
{
	return c;
}

//-----------------------------------------------------------------------------
inline real Sphere::get_radius() const
{
	return r;
}

//-----------------------------------------------------------------------------
inline real Sphere::get_volume() const
{
	return math::FOUR_OVER_THREE_TIMES_PI * r * r * r;
}

//-----------------------------------------------------------------------------
inline void Sphere::set_center(const Vec3& center)
{
	c = center;
}

//-----------------------------------------------------------------------------
inline void Sphere::set_radius(real radius)
{
	this->r = radius;
}

//-----------------------------------------------------------------------------
inline void Sphere::add_point(const Vec3& p)
{
	real dist = (p - c).squared_length();

	if (dist < r * r)
	{
		return;
	}

	r = math::sqrt(dist);
}

//-----------------------------------------------------------------------------
inline void Sphere::add_sphere(const Sphere& s)
{
	real dist = (s.c - c).squared_length();

	if (dist < (s.r + r) * (s.r + r))
		if (s.r * s.r > r * r)
		{
			r = math::sqrt(dist + s.r * s.r);
		}
}

//-----------------------------------------------------------------------------
inline bool Sphere::contains_point(const Vec3& p) const
{
	real dist = (p - c).squared_length();
	return (dist < r * r);
}

} // namespace crown

