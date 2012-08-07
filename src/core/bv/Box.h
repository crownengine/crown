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

#include <cassert>
#include "Types.h"
#include "Mat4.h"
#include "Vec3.h"
#include "Sphere.h"

namespace Crown
{

/**
	Axially aligned bounding box.

	Used mainly for collision detection and intersection tests.
*/
class Box
{

public:

	Vec3			min;
	Vec3			max;


					Box();													//!< Constructor
					Box(const Vec3& newMin, const Vec3& newMax);			//!< Constructs from min and max
					Box(const Box& box);									//!< Copy constructor
					~Box();													//!< Destructor

	const Vec3&		GetMin() const;											//!< Returns the "min" corner
	const Vec3&		GetMax() const;											//!< Returns the "max" corner
	void			SetMin(const Vec3& min);								//!< Sets the "min" corner
	void			SetMax(const Vec3& max);								//!< Sets the "max" corner

	Vec3			GetCenter() const;										//!< Returns the center
	real			GetRadius() const;										//!< Returns the radius
	real			GetVolume() const;										//!< Returns the volume

	void			AddPoint(const Vec3& p);								//!< Adds a point to the aabb
	void			AddBox(const Box& box);									//!< Adds a Box to the aabb

	bool			ContainsPoint(const Vec3& p) const;						//!< Returns whether point "p" is contained

	Vec3			GetVertex(uint index) const;							//!< Returns a box's vertex
	void			GetTransformed(const Mat4& mat, Box& result) const;		//!< Returns the box trasformed according to "mat" matrix

	void			ToVertices(Vec3 v[8]) const;							//!< Returns the eight box's vertices
	Sphere			ToSphere() const;										//!< Returns as a sphere

	void			Zero();													//!< Sets min and max to zero
};

//-----------------------------------------------------------------------------
inline Box::Box() : min(0.0, 0.0, 0.0), max(0.0, 0.0, 0.0)
{
}

//-----------------------------------------------------------------------------
inline Box::Box(const Box& box) : min(box.min), max(box.max)
{
}

//-----------------------------------------------------------------------------
inline Box::Box(const Vec3& newMin, const Vec3& newMax) : min(newMin), max(newMax)
{
}

//-----------------------------------------------------------------------------
inline Box::~Box()
{
}

//-----------------------------------------------------------------------------
inline const Vec3& Box::GetMin() const
{
	return min;
}

//-----------------------------------------------------------------------------
inline void Box::SetMin(const Vec3& min)
{
	this->min = min;
}

//-----------------------------------------------------------------------------
inline const Vec3& Box::GetMax() const
{
	return max;
}

//-----------------------------------------------------------------------------
inline void Box::SetMax(const Vec3& max)
{
	this->max = max;
}

//-----------------------------------------------------------------------------
inline void Box::AddPoint(const Vec3& p)
{
	if (p.x < min.x)
	{
		min.x = p.x;
	}

	if (p.y < min.y)
	{
		min.y = p.y;
	}

	if (p.z < min.z)
	{
		min.z = p.z;
	}

	if (p.x > max.x)
	{
		max.x = p.x;
	}

	if (p.y > max.y)
	{
		max.y = p.y;
	}

	if (p.z > max.z)
	{
		max.z = p.z;
	}
}

//-----------------------------------------------------------------------------
inline void Box::AddBox(const Box& box)
{
	if (box.min.x < min.x)
	{
		min.x = box.min.x;
	}

	if (box.min.y < min.y)
	{
		min.y = box.min.y;
	}

	if (box.min.z < min.z)
	{
		min.z = box.min.z;
	}

	if (box.max.x > max.x)
	{
		max.x = box.max.x;
	}

	if (box.max.y > max.y)
	{
		max.y = box.max.y;
	}

	if (box.max.z > max.z)
	{
		max.z = box.max.z;
	}
}

//-----------------------------------------------------------------------------
inline bool Box::ContainsPoint(const Vec3& p) const
{
	return (p.x > min.x && p.y > min.y && p.z > min.z &&
		p.x < max.x && p.y < max.y && p.z < max.z);
}

//-----------------------------------------------------------------------------
inline Vec3 Box::GetCenter() const
{
	return (min + max) * 0.5;
}

//-----------------------------------------------------------------------------
inline real Box::GetRadius() const
{
	return (max - (min + max) * 0.5).GetLength();
}

//-----------------------------------------------------------------------------
inline void Box::ToVertices(Vec3 v[8]) const
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
	v[0].x = min.x;
	v[0].y = min.y;
	v[0].z = max.z;

	v[1].x = max.x;
	v[1].y = min.y;
	v[1].z = max.z;

	v[2].x = max.x;
	v[2].y = min.y;
	v[2].z = min.z;

	v[3].x = min.x;
	v[3].y = min.y;
	v[3].z = min.z;

	v[4].x = min.x;
	v[4].y = max.y;
	v[4].z = max.z;

	v[5].x = max.x;
	v[5].y = max.y;
	v[5].z = max.z;

	v[6].x = max.x;
	v[6].y = max.y;
	v[6].z = min.z;

	v[7].x = min.x;
	v[7].y = max.y;
	v[7].z = min.z;
}

//-----------------------------------------------------------------------------
inline Vec3 Box::GetVertex(uint index) const
{
	assert(index < 8);

	switch (index)
	{
		case 0:
			return Vec3(min.x, min.y, min.z);
		case 1:
			return Vec3(max.x, min.y, min.z);
		case 2:
			return Vec3(max.x, min.y, max.z);
		case 3:
			return Vec3(min.x, min.y, max.z);
		case 4:
			return Vec3(min.x, max.y, min.z);
		case 5:
			return Vec3(max.x, max.y, min.z);
		case 6:
			return Vec3(max.x, max.y, max.z);
		case 7:
			return Vec3(min.x, max.y, max.z);
	}
}

//-----------------------------------------------------------------------------
inline void Box::GetTransformed(const Mat4& mat, Box& result) const
{
	Vec3 vertices[8];

	ToVertices(vertices);

	result.min = result.max = mat * vertices[0];

	for (uint i = 1; i < 8; i++)
	{
		vertices[i] = mat * vertices[i];
		result.AddPoint(vertices[i]);
	}
}

//-----------------------------------------------------------------------------
inline real Box::GetVolume() const
{
	return (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
}

//-----------------------------------------------------------------------------
inline void Box::Zero()
{
	min.Zero();
	max.Zero();
}

//-----------------------------------------------------------------------------
inline Sphere Box::ToSphere() const
{
	return Sphere(GetCenter(), GetRadius());
}

} // namespace Crown

