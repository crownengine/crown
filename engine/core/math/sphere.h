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
#include "math_utils.h"
#include "vector3.h"

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

	/// Constructs from @a center and @a radius.
	Sphere(const Vector3& center, float radius);
	Sphere(const Sphere& a);

	const Vector3& center() const;
	float radius() const;
	float volume() const;

	void set_c(const Vector3& center);
	void set_radius(float radius);

	/// Adds @a count @a points to the sphere expanding if necessary.
	void add_points(const Vector3* points, uint32_t count);

	/// Adds @a count @a spheres expanding if necessary.
	void add_spheres(const Sphere* spheres, uint32_t count);

	/// Returns whether point @a p is contained into the sphere.
	bool contains_point(const Vector3& p) const;

private:

	Vector3 _c;
	float _r;
};

inline Sphere::Sphere()
{
}

inline Sphere::Sphere(const Vector3& center, float radius) : _c(center), _r(radius)
{
}

inline Sphere::Sphere(const Sphere& a) : _c(a._c), _r(a._r)
{
}

inline const Vector3& Sphere::center() const
{
	return _c;
}

inline float Sphere::radius() const
{
	return _r;
}

inline float Sphere::volume() const
{
	return float(4.0 / 3.0 * math::PI) * _r * _r * _r;
}

inline void Sphere::set_c(const Vector3& center)
{
	_c = center;
}

inline void Sphere::set_radius(float radius)
{
	_r = radius;
}

inline void Sphere::add_points(const Vector3* points, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		const Vector3& p = points[i];

		float dist = vector3::squared_length(p - _c);

		if (dist >= _r * _r)
		{
			_r = math::sqrt(dist);
		}
	}
}

inline void Sphere::add_spheres(const Sphere* spheres, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		const Sphere& s = spheres[i];

		float dist = vector3::squared_length(s._c - _c);

		if (dist < (s._r + _r) * (s._r + _r))
		{
			if (s._r * s._r > _r * _r)
			{
				_r = math::sqrt(dist + s._r * s._r);
			}
		}
	}
}

inline bool Sphere::contains_point(const Vector3& p) const
{
	float dist = vector3::squared_length(p - _c);
	return (dist < _r * _r);
}

} // namespace crown
