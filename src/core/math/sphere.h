/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
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
		return float(4.0 / 3.0 * PI) * s.r*s.r*s.r;
	}

	inline void add_points(Sphere& s, uint32_t num, const Vector3* points)
	{
		for (uint32_t i = 0; i < num; ++i)
		{
			const float dist = vector3::squared_length(points[i] - s.c);
			if (dist >= s.r*s.r)
				s.r = sqrt(dist);
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
					s.r = sqrt(dist + spheres[i].r * spheres[i].r);
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
