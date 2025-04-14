/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/math/frustum.inl"

namespace crown
{
namespace frustum
{
	void from_matrix(Frustum &f, const Matrix4x4 &m, bool homogeneous_ndc, int handedness)
	{
		// See: https://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
		// Left plane.
		f.planes[0].n.x = m.x.w + m.x.y;
		f.planes[0].n.y = m.y.w + m.y.y;
		f.planes[0].n.z = m.z.w + m.z.y;
		f.planes[0].d = -(m.t.w + m.t.y);
		plane3::normalize(f.planes[0]);

		// Right plane.
		f.planes[1].n.x = m.x.w - m.x.x;
		f.planes[1].n.y = m.y.w - m.y.x;
		f.planes[1].n.z = m.z.w - m.z.x;
		f.planes[1].d = -(m.t.w - m.t.x);
		plane3::normalize(f.planes[1]);

		// Bottom plane.
		f.planes[2].n.x = m.x.w - m.x.y;
		f.planes[2].n.y = m.y.w - m.y.y;
		f.planes[2].n.z = m.z.w - m.z.y;
		f.planes[2].d = -(m.t.w - m.t.y);
		plane3::normalize(f.planes[2]);

		// Top plane.
		f.planes[3].n.x = m.x.w + m.x.x;
		f.planes[3].n.y = m.y.w + m.y.x;
		f.planes[3].n.z = m.z.w + m.z.x;
		f.planes[3].d = -(m.t.w + m.t.x);
		plane3::normalize(f.planes[3]);

		// Near plane.
		const f32 near_x = homogeneous_ndc ? m.x.w + m.x.z : m.x.z;
		const f32 near_y = homogeneous_ndc ? m.y.w + m.y.z : m.y.z;
		const f32 near_z = homogeneous_ndc ? m.z.w + m.z.z : m.z.z;
		const f32 near_d = homogeneous_ndc ? -(m.t.w + m.t.z) : -m.t.z;
		f.planes[4].n.x = handedness == 0 ? -near_x : near_x;
		f.planes[4].n.y = handedness == 0 ? -near_y : near_y;
		f.planes[4].n.z = handedness == 0 ? -near_z : near_z;
		f.planes[4].d   = handedness == 0 ? -near_d : near_d;
		plane3::normalize(f.planes[4]);

		// Far plane.
		const f32 far_x = m.x.w - m.x.z;
		const f32 far_y = m.y.w - m.y.z;
		const f32 far_z = m.z.w - m.z.z;
		const f32 far_d = -(m.t.w - m.t.z);
		f.planes[5].n.x = handedness == 0 ? -far_x : far_x;
		f.planes[5].n.y = handedness == 0 ? -far_y : far_y;
		f.planes[5].n.z = handedness == 0 ? -far_z : far_z;
		f.planes[5].d   = handedness == 0 ? -far_d : far_d;
		plane3::normalize(f.planes[5]);
	}

} // namespace frustum

} // namespace crown
