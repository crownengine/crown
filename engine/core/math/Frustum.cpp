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

#include "Frustum.h"
#include "Types.h"
#include "Intersection.h"
#include "Matrix4x4.h"
#include "AABB.h"
#include "Plane.h"

namespace crown
{

//-----------------------------------------------------------------------------
Frustum::Frustum()
{
}

//-----------------------------------------------------------------------------
Frustum::Frustum(const Frustum& frustum)
{
	m_planes[FrustumPlane::LEFT]	= frustum.m_planes[FrustumPlane::LEFT];
	m_planes[FrustumPlane::RIGHT]	= frustum.m_planes[FrustumPlane::RIGHT];
	m_planes[FrustumPlane::BOTTOM]	= frustum.m_planes[FrustumPlane::BOTTOM];
	m_planes[FrustumPlane::TOP]		= frustum.m_planes[FrustumPlane::TOP];
	m_planes[FrustumPlane::NEAR]	= frustum.m_planes[FrustumPlane::NEAR];
	m_planes[FrustumPlane::FAR]		= frustum.m_planes[FrustumPlane::FAR];
}

//-----------------------------------------------------------------------------
bool Frustum::contains_point(const Vector3& point) const
{
	if (plane::distance_to_point(m_planes[FrustumPlane::LEFT], point) < 0.0) return false;
	if (plane::distance_to_point(m_planes[FrustumPlane::RIGHT], point) < 0.0) return false;
	if (plane::distance_to_point(m_planes[FrustumPlane::BOTTOM], point) < 0.0) return false;
	if (plane::distance_to_point(m_planes[FrustumPlane::TOP], point) < 0.0) return false;
	if (plane::distance_to_point(m_planes[FrustumPlane::NEAR], point) < 0.0) return false;
	if (plane::distance_to_point(m_planes[FrustumPlane::FAR], point) < 0.0) return false;

	return true;
}

//-----------------------------------------------------------------------------
Vector3 Frustum::vertex(uint32_t index) const
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

	Vector3 ip;

	switch (index)
	{
		case 0: return Intersection::test_plane_3(m_planes[4], m_planes[0], m_planes[2], ip);
		case 1: return Intersection::test_plane_3(m_planes[4], m_planes[1], m_planes[2], ip);
		case 2: return Intersection::test_plane_3(m_planes[4], m_planes[1], m_planes[3], ip);
		case 3: return Intersection::test_plane_3(m_planes[4], m_planes[0], m_planes[3], ip);
		case 4: return Intersection::test_plane_3(m_planes[5], m_planes[0], m_planes[2], ip);
		case 5: return Intersection::test_plane_3(m_planes[5], m_planes[1], m_planes[2], ip);
		case 6: return Intersection::test_plane_3(m_planes[5], m_planes[1], m_planes[3], ip);
		case 7: return Intersection::test_plane_3(m_planes[5], m_planes[0], m_planes[3], ip);
		default: break;
	}

	return ip;
}

//-----------------------------------------------------------------------------
void Frustum::from_matrix(const Matrix4x4& m)
{
	// Left plane
	m_planes[FrustumPlane::LEFT].n.x	= m.m[3] + m.m[0];
	m_planes[FrustumPlane::LEFT].n.y	= m.m[7] + m.m[4];
	m_planes[FrustumPlane::LEFT].n.z	= m.m[11] + m.m[8];
	m_planes[FrustumPlane::LEFT].d		= m.m[15] + m.m[12];

	// Right plane
	m_planes[FrustumPlane::RIGHT].n.x	= m.m[3] - m.m[0];
	m_planes[FrustumPlane::RIGHT].n.y	= m.m[7] - m.m[4];
	m_planes[FrustumPlane::RIGHT].n.z	= m.m[11] - m.m[8];
	m_planes[FrustumPlane::RIGHT].d		= m.m[15] - m.m[12];

	// Bottom plane
	m_planes[FrustumPlane::BOTTOM].n.x	= m.m[3] + m.m[1];
	m_planes[FrustumPlane::BOTTOM].n.y	= m.m[7] + m.m[5];
	m_planes[FrustumPlane::BOTTOM].n.z	= m.m[11] + m.m[9];
	m_planes[FrustumPlane::BOTTOM].d	= m.m[15] + m.m[13];

	// Top plane
	m_planes[FrustumPlane::TOP].n.x		= m.m[3] - m.m[1];
	m_planes[FrustumPlane::TOP].n.y		= m.m[7] - m.m[5];
	m_planes[FrustumPlane::TOP].n.z		= m.m[11] - m.m[9];
	m_planes[FrustumPlane::TOP].d		= m.m[15] - m.m[13];

	// Near plane
	m_planes[FrustumPlane::NEAR].n.x	= m.m[3] + m.m[2];
	m_planes[FrustumPlane::NEAR].n.y	= m.m[7] + m.m[6];
	m_planes[FrustumPlane::NEAR].n.z	= m.m[11] + m.m[10];
	m_planes[FrustumPlane::NEAR].d		= m.m[15] + m.m[14];

	// Far plane
	m_planes[FrustumPlane::FAR].n.x		= m.m[3] - m.m[2];
	m_planes[FrustumPlane::FAR].n.y		= m.m[7] - m.m[6];
	m_planes[FrustumPlane::FAR].n.z		= m.m[11] - m.m[10];
	m_planes[FrustumPlane::FAR].d		= m.m[15] - m.m[14];

	plane::normalize(m_planes[FrustumPlane::LEFT]);
	plane::normalize(m_planes[FrustumPlane::RIGHT]);
	plane::normalize(m_planes[FrustumPlane::BOTTOM]);
	plane::normalize(m_planes[FrustumPlane::TOP]);
	plane::normalize(m_planes[FrustumPlane::NEAR]);
	plane::normalize(m_planes[FrustumPlane::FAR]);
}

//-----------------------------------------------------------------------------
AABB Frustum::to_aabb() const
{
	AABB tmp;
	aabb::reset(tmp);

	Vector3 vertices[8];
	vertices[0] = vertex(0);
	vertices[1] = vertex(1);
	vertices[2] = vertex(2);
	vertices[3] = vertex(3);
	vertices[4] = vertex(4);
	vertices[5] = vertex(5);
	vertices[6] = vertex(6);
	vertices[7] = vertex(7);

	aabb::add_points(tmp, 8, vertices);

	return tmp;
}

} // namespace crown
