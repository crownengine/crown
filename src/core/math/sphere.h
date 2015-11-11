/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "math_utils.h"
#include "vector3.h"

namespace crown
{
namespace sphere
{
	void reset(Sphere& s);

	float volume(const Sphere& s);

	/// Adds @a num @a points to the sphere @a s, expanding its bounds if necessary.
	void add_points(Sphere& s, uint32_t num, uint32_t stride, const void* points);

	/// Adds @a num @a points to the sphere expanding if necessary.
	void add_points(Sphere& s, uint32_t num, const Vector3* points);

	/// Adds @a num @a spheres expanding if necessary.
	void add_spheres(Sphere& s, uint32_t num, const Sphere* spheres);

	/// Returns whether point @a p is contained into the sphere.
	bool contains_point(const Sphere& s, const Vector3& p);
} // namespace sphere

namespace sphere
{
	inline void reset(Sphere& s)
	{
		s.c = VECTOR3_ZERO;
		s.r = 0.0f;
	}

	inline float volume(const Sphere& s)
	{
		return (4.0f/3.0f*PI) * (s.r*s.r*s.r);
	}

	inline void add_points(Sphere& s, uint32_t num, uint32_t stride, const void* points)
	{
		for (uint32_t i = 0; i < num; ++i)
		{
			const Vector3* p = (const Vector3*)points;

			const float dist = squared_length(*p - s.c);
			if (dist > s.r*s.r)
				s.r = sqrt(dist);

			points = (const void*)((const char*)points + stride);
		}
	}

	inline void add_points(Sphere& s, uint32_t num, const Vector3* points)
	{
		add_points(s, num, sizeof(Vector3), points);
	}

	inline void add_spheres(Sphere& s, uint32_t num, const Sphere* spheres)
	{
		for (uint32_t i = 0; i < num; ++i)
		{
			const float dist = squared_length(spheres[i].c - s.c);

			if (dist < (spheres[i].r + s.r) * (spheres[i].r + s.r))
			{
				if (spheres[i].r * spheres[i].r > s.r * s.r)
					s.r = sqrt(dist + spheres[i].r * spheres[i].r);
			}
		}
	}

	inline bool contains_point(const Sphere& s, const Vector3& p)
	{
		float dist = squared_length(p - s.c);
		return dist < s.r*s.r;
	}
} // namespace sphere

} // namespace crown
