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

#include "Types.h"
#include "MathTypes.h"
#include "Vector3.h"
#include "Plane.h"

#ifdef WINDOWS
#undef NEAR
#undef FAR
#endif

namespace crown
{

struct FrustumPlane
{
	enum Enum
	{
		LEFT	= 0,
		RIGHT	= 1,
		BOTTOM	= 2,
		TOP		= 3,
		NEAR	= 4,
		FAR		= 5
	};
};

struct Matrix4x4;

struct Frustum
{
	/// Does nothing for efficiency.
				Frustum();				
				Frustum(const Frustum& frustum);

	/// Returns whether @a point is contained into the frustum.
	bool		contains_point(const Vector3& point) const;	

	/// Returns one of the eight frustum's corners.
	Vector3		vertex(uint32_t index) const;			

	/// Builds the view frustum according to the matrix @a m.
	void		from_matrix(const Matrix4x4& m);				

	/// Returns a Box containing the frustum volume.
	AABB		to_aabb() const;							

public:

	Plane		m_planes[6];
};

} // namespace crown
