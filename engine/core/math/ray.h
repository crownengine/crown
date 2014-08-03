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

#include "types.h"
#include "vector3.h"

namespace crown
{

/// 3D Ray.
///
/// Defined by the origin and the unit-length direction vector.
struct Ray
{
public:

	/// Does nothing for efficiency.
					Ray();

	/// Constructs from @a origin and @a direction
					Ray(const Vector3& origin, const Vector3& direction);
					Ray(const Ray& ray);

	const Vector3&		origin() const;
	const Vector3&		direction() const;

	void			set_origin(const Vector3& origin);
	void			set_direction(const Vector3& direction);

private:

	Vector3			m_origin;
	Vector3			m_direction;
};

//-----------------------------------------------------------------------------
inline Ray::Ray()
{
}

//-----------------------------------------------------------------------------
inline Ray::Ray(const Vector3& origin, const Vector3& direction) : m_origin(origin), m_direction(direction)
{
}

//-----------------------------------------------------------------------------
inline Ray::Ray(const Ray& ray) : m_origin(ray.m_origin), m_direction(ray.m_direction)
{
}

//-----------------------------------------------------------------------------
inline const Vector3& Ray::origin() const
{
	return m_origin;
}

//-----------------------------------------------------------------------------
inline const Vector3& Ray::direction() const
{
	return m_direction;
}

//-----------------------------------------------------------------------------
inline void Ray::set_origin(const Vector3& origin)
{
	m_origin = origin;
}

//-----------------------------------------------------------------------------
inline void Ray::set_direction(const Vector3& direction)
{
	m_direction = direction;
}

} // namespace crown

