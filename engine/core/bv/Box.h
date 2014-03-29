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

#include "Assert.h"
#include "Types.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Sphere.h"

namespace crown
{

/// Axially aligned bounding box.
///
/// Used mainly for collision detection and intersection tests.
class Box
{
public:

	/// Does nothing for efficiency.
					Box();

	/// Constructs from @a min and @a max.
					Box(const Vector3& min, const Vector3& max);			
					Box(const Box& box);

	const Vector3&	min() const;
	const Vector3&	max() const;
	void			set_min(const Vector3& min);
	void			set_max(const Vector3& max);

	Vector3			center() const;
	float			radius() const;
	float			volume() const;

	/// Adds @a count @a points expanding if necessary.
	void			add_points(const Vector3* points, uint32_t count);

	/// Adds @a count @a boxes expanding if necessay.
	void			add_boxes(const Box* boxes, uint32_t count);

	/// Returns whether point @a p is contained in the box.
	bool			contains_point(const Vector3& p) const;

	/// Returns the @a index -th vertex of the box.
	Vector3			vertex(uint32_t index) const;		

	/// Returns the box trasformed according to @a mat matrix into @a result.
	void			transformed(const Matrix4x4& mat, Box& result) const;	

	/// Returns the eight vertices of the box.
	void			to_vertices(Vector3 v[8]) const;	

	/// Returns as a sphere.						
	Sphere			to_sphere() const;										

	/// Sets min and max to zero.
	void			zero();													

private:

	Vector3			m_min;
	Vector3			m_max;
};

//-----------------------------------------------------------------------------
inline Box::Box()
{
}

//-----------------------------------------------------------------------------
inline Box::Box(const Box& box) : m_min(box.m_min), m_max(box.m_max)
{
}

//-----------------------------------------------------------------------------
inline Box::Box(const Vector3& min, const Vector3& max) : m_min(min), m_max(max)
{
}

//-----------------------------------------------------------------------------
inline const Vector3& Box::min() const
{
	return m_min;
}

//-----------------------------------------------------------------------------
inline void Box::set_min(const Vector3& min)
{
	m_min = min;
}

//-----------------------------------------------------------------------------
inline const Vector3& Box::max() const
{
	return m_max;
}

//-----------------------------------------------------------------------------
inline void Box::set_max(const Vector3& max)
{
	m_max = max;
}

//-----------------------------------------------------------------------------
inline void Box::add_points(const Vector3* points, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		const Vector3& p = points[i];

		if (p.x < m_min.x)
		{
			m_min.x = p.x;
		}

		if (p.y < m_min.y)
		{
			m_min.y = p.y;
		}

		if (p.z < m_min.z)
		{
			m_min.z = p.z;
		}

		if (p.x > m_max.x)
		{
			m_max.x = p.x;
		}

		if (p.y > m_max.y)
		{
			m_max.y = p.y;
		}

		if (p.z > m_max.z)
		{
			m_max.z = p.z;
		}
	}
}

//-----------------------------------------------------------------------------
inline void Box::add_boxes(const Box* boxes, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		const Box& box = boxes[i];

		if (box.m_min.x < m_min.x)
		{
			m_min.x = box.m_min.x;
		}

		if (box.m_min.y < m_min.y)
		{
			m_min.y = box.m_min.y;
		}

		if (box.m_min.z < m_min.z)
		{
			m_min.z = box.m_min.z;
		}

		if (box.m_max.x > m_max.x)
		{
			m_max.x = box.m_max.x;
		}

		if (box.m_max.y > m_max.y)
		{
			m_max.y = box.m_max.y;
		}

		if (box.m_max.z > m_max.z)
		{
			m_max.z = box.m_max.z;
		}
	}
}

//-----------------------------------------------------------------------------
inline bool Box::contains_point(const Vector3& p) const
{
	return (p.x > m_min.x && p.y > m_min.y && p.z > m_min.z &&
		p.x < m_max.x && p.y < m_max.y && p.z < m_max.z);
}

//-----------------------------------------------------------------------------
inline Vector3 Box::center() const
{
	return (m_min + m_max) * 0.5;
}

//-----------------------------------------------------------------------------
inline float Box::radius() const
{
	return vector3::length(m_max - (m_min + m_max) * 0.5);
}

//-----------------------------------------------------------------------------
inline void Box::to_vertices(Vector3 v[8]) const
{
	// 7 ---- 6
	// |      |
	// |      |  <--- Top face
	// 4 ---- 5
	//
	// 3 ---- 2
	// |      |
	// |      |  <--- Bottom face
	// 0 ---- 1
	v[0].x = m_min.x;
	v[0].y = m_min.y;
	v[0].z = m_max.z;

	v[1].x = m_max.x;
	v[1].y = m_min.y;
	v[1].z = m_max.z;

	v[2].x = m_max.x;
	v[2].y = m_min.y;
	v[2].z = m_min.z;

	v[3].x = m_min.x;
	v[3].y = m_min.y;
	v[3].z = m_min.z;

	v[4].x = m_min.x;
	v[4].y = m_max.y;
	v[4].z = m_max.z;

	v[5].x = m_max.x;
	v[5].y = m_max.y;
	v[5].z = m_max.z;

	v[6].x = m_max.x;
	v[6].y = m_max.y;
	v[6].z = m_min.z;

	v[7].x = m_min.x;
	v[7].y = m_max.y;
	v[7].z = m_min.z;
}

//-----------------------------------------------------------------------------
inline Vector3 Box::vertex(uint32_t index) const
{
	CE_ASSERT(index < 8, "Index must be < 8");

	switch (index)
	{
		case 0:
			return Vector3(m_min.x, m_min.y, m_min.z);
		case 1:
			return Vector3(m_max.x, m_min.y, m_min.z);
		case 2:
			return Vector3(m_max.x, m_min.y, m_max.z);
		case 3:
			return Vector3(m_min.x, m_min.y, m_max.z);
		case 4:
			return Vector3(m_min.x, m_max.y, m_min.z);
		case 5:
			return Vector3(m_max.x, m_max.y, m_min.z);
		case 6:
			return Vector3(m_max.x, m_max.y, m_max.z);
		case 7:
			return Vector3(m_min.x, m_max.y, m_max.z);
	}
}

//-----------------------------------------------------------------------------
inline void Box::transformed(const Matrix4x4& mat, Box& result) const
{
	Vector3 vertices[8];

	to_vertices(vertices);

	result.m_min = mat * vertices[0];
	result.m_max = mat * vertices[0];

	vertices[1] = mat * vertices[1];
	vertices[2] = mat * vertices[2];
	vertices[3] = mat * vertices[3];
	vertices[4] = mat * vertices[4];
	vertices[5] = mat * vertices[5];
	vertices[6] = mat * vertices[6];
	vertices[7] = mat * vertices[7];

	result.add_points(&vertices[1], 7);
}

//-----------------------------------------------------------------------------
inline float Box::volume() const
{
	return (m_max.x - m_min.x) * (m_max.y - m_min.y) * (m_max.z - m_min.z);
}

//-----------------------------------------------------------------------------
inline void Box::zero()
{
	m_min = vector3::ZERO;
	m_max = vector3::ZERO;
}

//-----------------------------------------------------------------------------
inline Sphere Box::to_sphere() const
{
	return Sphere(center(), radius());
}

} // namespace crown

