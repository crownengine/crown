/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
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

	void split(Frustum *splits, u32 num, const Frustum &f, f32 weight, f32 overlap)
	{
		CE_ENSURE(num > 1);

		// See: https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf
		const f32 ratio = fabs(f.planes[5].d / f.planes[4].d);
		const f32 diff  = fabs(f.planes[5].d - f.planes[4].d);

		splits[0].planes[0] = f.planes[0];
		splits[0].planes[1] = f.planes[1];
		splits[0].planes[2] = f.planes[2];
		splits[0].planes[3] = f.planes[3];
		splits[0].planes[4] = f.planes[4];

		for (u32 i = 1; i < num; ++i) {
			const f32 si = i / f32(num);

			splits[i].planes[0]   = f.planes[0];
			splits[i].planes[1]   = f.planes[1];
			splits[i].planes[2]   = f.planes[2];
			splits[i].planes[3]   = f.planes[3];
			splits[i].planes[4].n = f.planes[4].n;
			splits[i].planes[4].d = weight * (f.planes[4].d * fpow(ratio, si)) + (1 - weight) * (f.planes[4].d + diff * si);
			splits[i].planes[5].n = f.planes[5].n;

			splits[i - 1].planes[5].n = f.planes[5].n;
			splits[i - 1].planes[5].d = -splits[i].planes[4].d * overlap;
		}

		splits[num - 1].planes[5] = f.planes[5];
	}

} // namespace frustum

} // namespace crown
