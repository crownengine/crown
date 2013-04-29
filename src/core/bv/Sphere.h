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


/// Sphere.
///
/// Used mainly for collision detection and intersection tests.
class Sphere
{
public:

	/// Does nothing for efficiency.
					Sphere();

	/// Constructs from center and radius.
					Sphere(const Vec3& center, real radius);
					Sphere(const Sphere& a);

	const Vec3&		center() const;		
	real			radius() const;	
	real			volume() const;	

	void			set_center(const Vec3& center);
	void			set_radius(real radius);

	/// Adds a point expanding if necessary.
	void			add_points(const Vec3* points, uint32_t count);	

	/// Adds a sphere expanding if necessary.
	void			add_spheres(const Sphere* spheres, uint32_t count);	

	/// Returns whether point @p is contained.
	bool			contains_point(const Vec3& p) const;		

private:

	Vec3			m_center;
	real			m_radius;
};

//-----------------------------------------------------------------------------
inline Sphere::Sphere()
{
}

//-----------------------------------------------------------------------------
inline Sphere::Sphere(const Vec3& center, real radius) : m_center(center), m_radius(radius)
{
}

//-----------------------------------------------------------------------------
inline Sphere::Sphere(const Sphere& a) : m_center(a.m_center), m_radius(a.m_radius)
{
}

//-----------------------------------------------------------------------------
inline const Vec3& Sphere::center() const
{
	return m_center;
}

//-----------------------------------------------------------------------------
inline real Sphere::radius() const
{
	return m_radius;
}

//-----------------------------------------------------------------------------
inline real Sphere::volume() const
{
	return math::FOUR_OVER_THREE_TIMES_PI * m_radius * m_radius * m_radius;
}

//-----------------------------------------------------------------------------
inline void Sphere::set_center(const Vec3& center)
{
	m_center = center;
}

//-----------------------------------------------------------------------------
inline void Sphere::set_radius(real radius)
{
	m_radius = radius;
}

//-----------------------------------------------------------------------------
inline void Sphere::add_points(const Vec3* points, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		const Vec3& p = points[i];

		real dist = (p - m_center).squared_length();

		if (dist >= m_radius * m_radius)
		{
			m_radius = math::sqrt(dist);
		}
	}
}

//-----------------------------------------------------------------------------
inline void Sphere::add_spheres(const Sphere* spheres, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		const Sphere& s = spheres[i];

		real dist = (s.m_center - m_center).squared_length();

		if (dist < (s.m_radius + m_radius) * (s.m_radius + m_radius))
		{
			if (s.m_radius * s.m_radius > m_radius * m_radius)
			{
				m_radius = math::sqrt(dist + s.m_radius * s.m_radius);
			}
		}
	}
}

//-----------------------------------------------------------------------------
inline bool Sphere::contains_point(const Vec3& p) const
{
	real dist = (p - m_center).squared_length();
	return (dist < m_radius * m_radius);
}

} // namespace crown

