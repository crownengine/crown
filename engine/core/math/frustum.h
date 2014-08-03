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
#include "math_types.h"
#include "vector3.h"
#include "plane.h"
#include "intersection.h"

namespace crown
{

/// Functions to manipulate Frustum.
///
/// @ingroup Math
namespace frustum
{
	/// Returns whether the frustum @a f contains the point @a p.
	bool contains_point(const Frustum& f, const Vector3& p);

	/// Returns the corner @a index of the frustum @a f.
	/// @note
	/// Index to corner table:
	/// 0 = Near bottom left
	/// 1 = Near bottom right
	/// 2 = Near top right
	/// 3 = Near top left
	/// 4 = Far bottom left
	/// 5 = Far bottom right
	/// 6 = Far top right
	/// 7 = Far top left
	Vector3 vertex(const Frustum& f, uint32_t index);

	/// Builds the frustum @a f from the view matrix @a m.
	void from_matrix(Frustum& f, const Matrix4x4& m);

	/// Returns the AABB enclosing the frustum @a f.
	AABB to_aabb(const Frustum& f);
} // namespace frustum

namespace frustum
{
	//-----------------------------------------------------------------------------
	inline bool contains_point(const Frustum& f, const Vector3& p)
	{
		if (plane::distance_to_point(f.left, p) < 0.0) return false;
		if (plane::distance_to_point(f.right, p) < 0.0) return false;
		if (plane::distance_to_point(f.bottom, p) < 0.0) return false;
		if (plane::distance_to_point(f.top, p) < 0.0) return false;
		if (plane::distance_to_point(f.near, p) < 0.0) return false;
		if (plane::distance_to_point(f.far, p) < 0.0) return false;

		return true;
	}

	//-----------------------------------------------------------------------------
	inline Vector3 vertex(const Frustum& f, uint32_t index)
	{
		CE_ASSERT(index < 8, "Index must be < 8");

		// 0 = Near bottom left
		// 1 = Near bottom right
		// 2 = Near top right
		// 3 = Near top left
		// 4 = Far bottom left
		// 5 = Far bottom right
		// 6 = Far top right
		// 7 = Far top left

		const Plane* side = &f.left;
		Vector3 ip;

		switch (index)
		{
			case 0: return Intersection::test_plane_3(side[4], side[0], side[2], ip);
			case 1: return Intersection::test_plane_3(side[4], side[1], side[2], ip);
			case 2: return Intersection::test_plane_3(side[4], side[1], side[3], ip);
			case 3: return Intersection::test_plane_3(side[4], side[0], side[3], ip);
			case 4: return Intersection::test_plane_3(side[5], side[0], side[2], ip);
			case 5: return Intersection::test_plane_3(side[5], side[1], side[2], ip);
			case 6: return Intersection::test_plane_3(side[5], side[1], side[3], ip);
			case 7: return Intersection::test_plane_3(side[5], side[0], side[3], ip);
			default: break;
		}

		return ip;
	}

	//-----------------------------------------------------------------------------
	inline void from_matrix(Frustum& f, const Matrix4x4& m)
	{
		f.left.n.x		= m[3] + m[0];
		f.left.n.y		= m[7] + m[4];
		f.left.n.z		= m[11] + m[8];
		f.left.d		= m[15] + m[12];

		f.right.n.x		= m[3] - m[0];
		f.right.n.y		= m[7] - m[4];
		f.right.n.z		= m[11] - m[8];
		f.right.d		= m[15] - m[12];

		f.bottom.n.x	= m[3] + m[1];
		f.bottom.n.y	= m[7] + m[5];
		f.bottom.n.z	= m[11] + m[9];
		f.bottom.d		= m[15] + m[13];

		f.top.n.x		= m[3] - m[1];
		f.top.n.y		= m[7] - m[5];
		f.top.n.z		= m[11] - m[9];
		f.top.d			= m[15] - m[13];

		f.near.n.x		= m[3] + m[2];
		f.near.n.y		= m[7] + m[6];
		f.near.n.z		= m[11] + m[10];
		f.near.d		= m[15] + m[14];

		f.far.n.x		= m[3] - m[2];
		f.far.n.y		= m[7] - m[6];
		f.far.n.z		= m[11] - m[10];
		f.far.d			= m[15] - m[14];

		plane::normalize(f.left);
		plane::normalize(f.right);
		plane::normalize(f.bottom);
		plane::normalize(f.top);
		plane::normalize(f.near);
		plane::normalize(f.far);
	}

	//-----------------------------------------------------------------------------
	inline AABB to_aabb(const Frustum& f)
	{
		AABB tmp;
		aabb::reset(tmp);

		Vector3 vertices[8];
		vertices[0] = vertex(f, 0);
		vertices[1] = vertex(f, 1);
		vertices[2] = vertex(f, 2);
		vertices[3] = vertex(f, 3);
		vertices[4] = vertex(f, 4);
		vertices[5] = vertex(f, 5);
		vertices[6] = vertex(f, 6);
		vertices[7] = vertex(f, 7);

		aabb::add_points(tmp, 8, vertices);

		return tmp;
	}
} // namespace frustum

//-----------------------------------------------------------------------------
inline Frustum::Frustum()
{
	// Do not initialize
}

} // namespace crown
