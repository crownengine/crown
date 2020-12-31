/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/math.h"
#include "core/math/sphere.h"
#include "core/math/vector3.inl"

namespace crown
{
namespace sphere
{
	inline f32 volume(const Sphere& s)
	{
		return (4.0f/3.0f*PI) * (s.r*s.r*s.r);
	}

	inline void add_points(Sphere& s, u32 num, const Vector3* points)
	{
		add_points(s, num, sizeof(Vector3), points);
	}

	inline bool contains_point(const Sphere& s, const Vector3& p)
	{
		f32 dist = length_squared(p - s.c);
		return dist < s.r*s.r;
	}

} // namespace sphere

} // namespace crown
