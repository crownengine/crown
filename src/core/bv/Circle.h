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
#include "Vec2.h"
#include "MathUtils.h"

namespace Crown
{

class Rect;

/**
	Circle.

	Used mainly for collision detection and intersection tests.
*/
class Circle
{

public:

	Vec2			c;
	real			r;

					Circle();									//!< Constructor, does nothing for efficiency
					Circle(const Vec2& center, real radius);	//!< Constructs from center and radius
					Circle(const Circle& circle);				//!< Copy constructor
					~Circle();									//!< Destructor

	const Vec2&		GetCenter() const;							//!< Returns the center
	real			GetRadius() const;							//!< Returns the radius
	void			SetCenter(const Vec2& center);				//!< Sets the center
	void			SetRadius(real radius);						//!< Sets the radius

	real			GetArea() const;							//!< Returns the area

	Rect			ToRect() const;								//!< Returns the equivalent rect
};

//-----------------------------------------------------------------------------
inline Circle::Circle()
{
}

//-----------------------------------------------------------------------------
inline Circle::Circle(const Vec2& c, real r) : c(c), r(r)
{
}

//-----------------------------------------------------------------------------
inline Circle::Circle(const Circle& circle) : c(circle.c), r(circle.r)
{
}

//-----------------------------------------------------------------------------
inline Circle::~Circle()
{
}

//-----------------------------------------------------------------------------
inline const Vec2& Circle::GetCenter() const
{
	return c;
}

//-----------------------------------------------------------------------------
inline real Circle::GetRadius() const
{
	return r;
}

//-----------------------------------------------------------------------------
inline void Circle::SetCenter(const Vec2& center)
{
	c = center;
}

//-----------------------------------------------------------------------------
inline void Circle::SetRadius(real radius)
{
	r = radius;
}

//-----------------------------------------------------------------------------
inline real Circle::GetArea() const
{
	return r * r * Math::PI;
}

} // namespace Crown

