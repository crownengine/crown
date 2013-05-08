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

#pragma once

#include "Types.h"
#include "Box.h"
#include "Vec3.h"
#include "Plane.h"

namespace crown
{

enum FrustumPlane
{
	FP_LEFT		= 0,
	FP_RIGHT	= 1,
	FP_BOTTOM	= 2,
	FP_TOP		= 3,
	FP_NEAR		= 4,
	FP_FAR		= 5
};

class Mat4;

class Frustum
{
public:

	/// Does nothing for efficiency.
				Frustum();				
				Frustum(const Frustum& frustum);

	/// Returns whether @point is contained into the frustum.
	bool		contains_point(const Vec3& point) const;	

	/// Returns one of the eight frustum's corners.
	Vec3		vertex(uint32_t index) const;			

	/// Builds the view frustum according to the matrix @m.
	void		from_matrix(const Mat4& m);				

	/// Returns a Box containing the frustum volume.
	Box			to_box() const;							

private:

	Plane		m_planes[6];

	friend class Intersection;
};

} // namespace crown

