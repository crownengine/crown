/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"
#include "math_utils.h"
#include "vector3.h"

namespace crown
{
/// Functions to manipulate Sphere.
///
/// @ingroup Math
namespace sphere
{
	/// Resets the sphere @a s.
	void reset(Sphere& s);

	/// Returns the volume of the sphere @a s.
	f32 volume(const Sphere& s);

	/// Adds @a num @a points to the sphere @a s, expanding its bounds if necessary.
	void add_points(Sphere& s, u32 num, u32 stride, const void* points);

	/// Adds @a num @a points to the sphere expanding if necessary.
	void add_points(Sphere& s, u32 num, const Vector3* points);

	/// Adds @a num @a spheres expanding if necessary.
	void add_spheres(Sphere& s, u32 num, const Sphere* spheres);

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

	inline f32 volume(const Sphere& s)
	{
		return (4.0f/3.0f*PI) * (s.r*s.r*s.r);
	}

	inline void add_points(Sphere& s, u32 num, u32 stride, const void* points)
	{
		for (u32 i = 0; i < num; ++i)
		{
			const Vector3* p = (const Vector3*)points;

			const f32 dist = length_squared(*p - s.c);
			if (dist > s.r*s.r)
				s.r = sqrtf(dist);

			points = (const void*)((const char*)points + stride);
		}
	}

	inline void add_points(Sphere& s, u32 num, const Vector3* points)
	{
		add_points(s, num, sizeof(Vector3), points);
	}

	inline void add_spheres(Sphere& s, u32 num, const Sphere* spheres)
	{
		for (u32 i = 0; i < num; ++i)
		{
			const Sphere si = spheres[i];
			const f32 dist = length_squared(si.c - s.c);

			if (dist < (si.r + s.r) * (si.r + s.r))
			{
				if (si.r*si.r > s.r*s.r)
					s.r = sqrtf(dist + si.r*si.r);
			}
		}
	}

	inline bool contains_point(const Sphere& s, const Vector3& p)
	{
		f32 dist = length_squared(p - s.c);
		return dist < s.r*s.r;
	}
} // namespace sphere

} // namespace crown
