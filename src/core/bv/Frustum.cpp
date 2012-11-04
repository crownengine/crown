/*
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

namespace Crown
{

//-----------------------------------------------------------------------------
Frustum::Frustum()
{
	mPlane[FP_LEFT]		= Plane::ZERO;
	mPlane[FP_RIGHT]	= Plane::ZERO;
	mPlane[FP_BOTTOM]	= Plane::ZERO;
	mPlane[FP_TOP]		= Plane::ZERO;
	mPlane[FP_NEAR]		= Plane::ZERO;
	mPlane[FP_FAR]		= Plane::ZERO;
}

//-----------------------------------------------------------------------------
Frustum::Frustum(const Frustum& frustum)
{
	mPlane[FP_LEFT]		= frustum.mPlane[FP_LEFT];
	mPlane[FP_RIGHT]	= frustum.mPlane[FP_RIGHT];
	mPlane[FP_BOTTOM]	= frustum.mPlane[FP_BOTTOM];
	mPlane[FP_TOP]		= frustum.mPlane[FP_TOP];
	mPlane[FP_NEAR]		= frustum.mPlane[FP_NEAR];
	mPlane[FP_FAR]		= frustum.mPlane[FP_FAR];
}

//-----------------------------------------------------------------------------
Frustum::~Frustum()
{
}

//-----------------------------------------------------------------------------
bool Frustum::contains_point(const Vec3& point) const
{
	if (mPlane[FP_LEFT].get_distance_to_point(point) < 0.0) return false;
	if (mPlane[FP_RIGHT].get_distance_to_point(point) < 0.0) return false;
	if (mPlane[FP_BOTTOM].get_distance_to_point(point) < 0.0) return false;
	if (mPlane[FP_TOP].get_distance_to_point(point) < 0.0) return false;
	if (mPlane[FP_NEAR].get_distance_to_point(point) < 0.0) return false;
	if (mPlane[FP_FAR].get_distance_to_point(point) < 0.0) return false;

	return true;
}

//-----------------------------------------------------------------------------
Vec3 Frustum::get_vertex(uint index) const
{
	// 0 = Near bottom left
	// 1 = Near bottom right
	// 2 = Near top right
	// 3 = Near top left
	// 4 = Far bottom left
	// 5 = Far bottom right
	// 6 = Far top right
	// 7 = Far top left

	Vec3 ip(0, 0, 0);

	switch (index)
	{
		case 0:
			Intersection::TestPlane3(mPlane[4], mPlane[0], mPlane[2], ip);
		case 1:
			Intersection::TestPlane3(mPlane[4], mPlane[1], mPlane[2], ip);
		case 2:
			Intersection::TestPlane3(mPlane[4], mPlane[1], mPlane[3], ip);
		case 3:
			Intersection::TestPlane3(mPlane[4], mPlane[0], mPlane[3], ip);
		case 4:
			Intersection::TestPlane3(mPlane[5], mPlane[0], mPlane[2], ip);
		case 5:
			Intersection::TestPlane3(mPlane[5], mPlane[1], mPlane[2], ip);
		case 6:
			Intersection::TestPlane3(mPlane[5], mPlane[1], mPlane[3], ip);
		case 7:
			Intersection::TestPlane3(mPlane[5], mPlane[0], mPlane[3], ip);
		default:
			return ip;
	}

	return ip;
}

//-----------------------------------------------------------------------------
void Frustum::from_matrix(const Mat4& m)
{
	// Left plane
	mPlane[FP_LEFT].n.x		= m.m[3] + m.m[0];
	mPlane[FP_LEFT].n.y		= m.m[7] + m.m[4];
	mPlane[FP_LEFT].n.z		= m.m[11] + m.m[8];
	mPlane[FP_LEFT].d		= m.m[15] + m.m[12];

	// Right plane
	mPlane[FP_RIGHT].n.x	= m.m[3] - m.m[0];
	mPlane[FP_RIGHT].n.y	= m.m[7] - m.m[4];
	mPlane[FP_RIGHT].n.z	= m.m[11] - m.m[8];
	mPlane[FP_RIGHT].d		= m.m[15] - m.m[12];

	// Bottom plane
	mPlane[FP_BOTTOM].n.x	= m.m[3] + m.m[1];
	mPlane[FP_BOTTOM].n.y	= m.m[7] + m.m[5];
	mPlane[FP_BOTTOM].n.z	= m.m[11] + m.m[9];
	mPlane[FP_BOTTOM].d		= m.m[15] + m.m[13];

	// Top plane
	mPlane[FP_TOP].n.x		= m.m[3] - m.m[1];
	mPlane[FP_TOP].n.y		= m.m[7] - m.m[5];
	mPlane[FP_TOP].n.z		= m.m[11] - m.m[9];
	mPlane[FP_TOP].d		= m.m[15] - m.m[13];

	// Near plane
	mPlane[FP_NEAR].n.x		= m.m[3] + m.m[2];
	mPlane[FP_NEAR].n.y		= m.m[7] + m.m[6];
	mPlane[FP_NEAR].n.z		= m.m[11] + m.m[10];
	mPlane[FP_NEAR].d		= m.m[15] + m.m[14];

	// Far plane
	mPlane[FP_FAR].n.x		= m.m[3] - m.m[2];
	mPlane[FP_FAR].n.y		= m.m[7] - m.m[6];
	mPlane[FP_FAR].n.z		= m.m[11] - m.m[10];
	mPlane[FP_FAR].d		= m.m[15] - m.m[14];

	mPlane[FP_LEFT].normalize();
	mPlane[FP_RIGHT].normalize();
	mPlane[FP_BOTTOM].normalize();
	mPlane[FP_TOP].normalize();
	mPlane[FP_NEAR].normalize();
	mPlane[FP_FAR].normalize();
}

//-----------------------------------------------------------------------------
Box Frustum::to_box() const
{
	Box tmp;
	tmp.zero();

	tmp.add_point(this->get_vertex(0));
	tmp.add_point(this->get_vertex(1));
	tmp.add_point(this->get_vertex(2));
	tmp.add_point(this->get_vertex(3));
	tmp.add_point(this->get_vertex(4));
	tmp.add_point(this->get_vertex(5));
	tmp.add_point(this->get_vertex(6));
	tmp.add_point(this->get_vertex(7));

	return tmp;
}

} // namespace Crown

