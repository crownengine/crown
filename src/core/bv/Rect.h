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
#include "Vec2.h"


namespace Crown
{

class Circle;

/**
	2D rectangle.

	Used mainly for collision detection and intersection tests.
*/
class Rect
{

public:

	Vec2			min;
	Vec2			max;

					Rect();									//!< Constructor
					Rect(const Vec2& a, const Vec2& b);		//!< Builds from "a" min value and "b" max value
					Rect(const Rect& rect);					//!< Copy construcor
					~Rect();								//!< Destructor

	const Vec2&		get_min() const;						//!< Returns the "min" corner
	const Vec2&		get_max() const;						//!< Returns the "max" corner
	void			set_min(const Vec2& min);				//!< Sets the "min" corner
	void			set_max(const Vec2& max);				//!< Sets the "max" corner

	Vec2			get_center() const;						//!< Returns the center
	real			get_radius() const;						//!< Returns the radius 
	real			get_area() const;						//!< Returns the area
	Vec2			get_size() const;						//!< Returns the diagonal

	bool			contains_point(const Vec2& point) const;//!< Returns whether "point" is contained
	bool			intersects_rect(const Rect& r) const;	//!< Returns whether intersects "r"
	void			set_from_center_and_dimensions(Vec2 center, real width, real height);	//!< Sets the Rect from a center and a width - height
	void			get_vertices(Vec2 v[4]) const;			//!< Returns the four rect's vertices
	Vec2			get_vertex(uint index) const;			//!< Returns a rect's vertex

	Circle			to_circle() const;						//!< Returns the equivalent circle

	void			fix();									//!< Ensures that min and max aren't swapped
};

//-----------------------------------------------------------------------------
inline Rect::Rect()
{
}

//-----------------------------------------------------------------------------
inline Rect::Rect(const Vec2& a, const Vec2& b) : min(a), max(b)
{
}

//-----------------------------------------------------------------------------
inline Rect::Rect(const Rect& rect) : min(rect.min), max(rect.max)
{
}

//-----------------------------------------------------------------------------
inline Rect::~Rect()
{
}

//-----------------------------------------------------------------------------
inline const Vec2& Rect::get_min() const
{
	return min;
}

//-----------------------------------------------------------------------------
inline const Vec2& Rect::get_max() const
{
	return max;
}

//-----------------------------------------------------------------------------
inline void Rect::set_min(const Vec2& min)
{
	this->min = min;
}

//-----------------------------------------------------------------------------
inline void Rect::set_max(const Vec2& max)
{
	this->max = max;
}

} // namespace Crown

