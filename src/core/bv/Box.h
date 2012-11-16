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

namespace crown
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

	const Vec3&		get_min() const;										//!< Returns the "min" corner
	const Vec3&		get_max() const;										//!< Returns the "max" corner
	void			set_min(const Vec3& min);								//!< Sets the "min" corner
	void			set_max(const Vec3& max);								//!< Sets the "max" corner

	Vec3			get_center() const;										//!< Returns the center
	real			get_radius() const;										//!< Returns the radius
	real			get_volume() const;										//!< Returns the volume

	void			add_point(const Vec3& p);								//!< Adds a point to the aabb
	void			add_box(const Box& box);								//!< Adds a Box to the aabb

	bool			contains_point(const Vec3& p) const;					//!< Returns whether point "p" is contained

	Vec3			get_vertex(uint32_t index) const;							//!< Returns a box's vertex
	void			get_transformed(const Mat4& mat, Box& result) const;	//!< Returns the box trasformed according to "mat" matrix

	void			to_vertices(Vec3 v[8]) const;							//!< Returns the eight box's vertices
	Sphere			to_sphere() const;										//!< Returns as a sphere

	void			zero();													//!< Sets min and max to zero
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
inline const Vec3& Box::get_min() const
{
	return min;
}

//-----------------------------------------------------------------------------
inline void Box::set_min(const Vec3& min)
{
	this->min = min;
}

//-----------------------------------------------------------------------------
inline const Vec3& Box::get_max() const
{
	return max;
}

//-----------------------------------------------------------------------------
inline void Box::set_max(const Vec3& max)
{
	this->max = max;
}

//-----------------------------------------------------------------------------
inline void Box::add_point(const Vec3& p)
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
inline void Box::add_box(const Box& box)
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
inline bool Box::contains_point(const Vec3& p) const
{
	return (p.x > min.x && p.y > min.y && p.z > min.z &&
		p.x < max.x && p.y < max.y && p.z < max.z);
}

//-----------------------------------------------------------------------------
inline Vec3 Box::get_center() const
{
	return (min + max) * 0.5;
}

//-----------------------------------------------------------------------------
inline real Box::get_radius() const
{
	return (max - (min + max) * 0.5).length();
}

//-----------------------------------------------------------------------------
inline void Box::to_vertices(Vec3 v[8]) const
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
inline Vec3 Box::get_vertex(uint32_t index) const
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
inline void Box::get_transformed(const Mat4& mat, Box& result) const
{
	Vec3 vertices[8];

	to_vertices(vertices);

	result.min = result.max = mat * vertices[0];

	for (uint32_t i = 1; i < 8; i++)
	{
		vertices[i] = mat * vertices[i];
		result.add_point(vertices[i]);
	}
}

//-----------------------------------------------------------------------------
inline real Box::get_volume() const
{
	return (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
}

//-----------------------------------------------------------------------------
inline void Box::zero()
{
	min.zero();
	max.zero();
}

//-----------------------------------------------------------------------------
inline Sphere Box::to_sphere() const
{
	return Sphere(get_center(), get_radius());
}

} // namespace crown

