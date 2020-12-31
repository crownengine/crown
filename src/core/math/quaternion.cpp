/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/math/quaternion.inl"
#include "core/types.h"

namespace crown
{
/// Returns the rotation portion of the matrix @a m as a Quaternion.
Quaternion quaternion(const Matrix3x3& m)
{
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
	const f32 tr = m.x.x + m.y.y + m.z.z;

	Quaternion tmp;
	if (tr > 0.0f)
	{
		const f32 sq = fsqrt(1.0f + tr) * 0.5f;
		const f32 inv = 0.25f / sq;
		tmp.w = sq;
		tmp.x = (m.y.z - m.z.y) * inv;
		tmp.y = (m.z.x - m.x.z) * inv;
		tmp.z = (m.x.y - m.y.x) * inv;
	}
	else if ((m.x.x > m.y.y) && (m.x.x > m.z.z))
	{
		const f32 sq = fsqrt(1.0f + m.x.x - m.y.y - m.z.z) * 0.5f;
		const f32 inv = 0.25f / sq;
		tmp.x = sq;
		tmp.w = (m.y.z - m.z.y) * inv;
		tmp.y = (m.x.y + m.y.x) * inv;
		tmp.z = (m.z.x + m.x.z) * inv;
	}
	else if (m.y.y > m.z.z)
	{
		const f32 sq = fsqrt(1.0f + m.y.y - m.x.x - m.z.z) * 0.5f;
		const f32 inv = 0.25f / sq;
		tmp.y = sq;
		tmp.w = (m.z.x - m.x.z) * inv;
		tmp.x = (m.x.y + m.y.x) * inv;
		tmp.z = (m.y.z + m.z.y) * inv;
	}
	else
	{
		const f32 sq = fsqrt(1.0f + m.z.z - m.x.x - m.y.y) * 0.5f;
		const f32 inv = 0.25f / sq;
		tmp.z = sq;
		tmp.w = (m.x.y - m.y.x) * inv;
		tmp.x = (m.z.x + m.x.z) * inv;
		tmp.y = (m.y.z + m.z.y) * inv;
	}
	return tmp;
}

} // namespace crown
