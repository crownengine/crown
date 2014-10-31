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
#include "math_types.h"
#include "vector3.h"

namespace crown
{
namespace sphere
{
	float volume(const Sphere& s);

	/// Adds @a num @a points to the sphere expanding if necessary.
	void add_points(Sphere& s, uint32_t num, const Vector3* points);

	/// Adds @a num @a spheres expanding if necessary.
	void add_spheres(Sphere& s, uint32_t num, const Sphere* spheres);

	/// Returns whether point @a p is contained into the sphere.
	bool contains_point(const Sphere& s, const Vector3& p);
} // namespace sphere

namespace sphere
{
	inline float volume(const Sphere& s)
	{
		return float(4.0 / 3.0 * math::PI) * s.r*s.r*s.r;
	}

	inline void add_points(Sphere& s, uint32_t num, const Vector3* points)
	{
		for (uint32_t i = 0; i < num; ++i)
		{
			const float dist = vector3::squared_length(points[i] - s.c);
			if (dist >= s.r*s.r)
				s.r = math::sqrt(dist);
		}
	}

	inline void add_spheres(Sphere& s, uint32_t num, const Sphere* spheres)
	{
		for (uint32_t i = 0; i < num; ++i)
		{
			const float dist = vector3::squared_length(spheres[i].c - s.c);

			if (dist < (spheres[i].r + s.r) * (spheres[i].r + s.r))
			{
				if (spheres[i].r * spheres[i].r > s.r * s.r)
					s.r = math::sqrt(dist + spheres[i].r * spheres[i].r);
			}
		}
	}

	inline bool contains_point(const Sphere& s, const Vector3& p)
	{
		float dist = vector3::squared_length(p - s.c);
		return dist < s.r*s.r;
	}
} // namespace sphere

inline Sphere::Sphere()
{
	// Do nothing
}

inline Sphere::Sphere(const Vector3& nc, float nr)
	: c(nc)
	, r(nr)
{
}

} // namespace crown
