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
#include "Vector2.h"
#include "MathUtils.h"

namespace crown
{

class Rect;

/// Circle.
///
/// Used mainly for collision detection and intersection tests.
class Circle
{
public:

	/// Does nothing for efficiency.
					Circle();
					
	/// Constructs from @a center and @a radius.
					Circle(const Vector2& center, float radius);	
					Circle(const Circle& circle);				

	const Vector2&		center() const;							
	float			radius() const;	
	void			set_center(const Vector2& center);			
	void			set_radius(float radius);				

	float			area() const;						

	/// Returns a Rect containing the circle.
	Rect			to_rect() const;

private:

	Vector2			m_center;
	float			m_radius;
};

//-----------------------------------------------------------------------------
inline Circle::Circle()
{
}

//-----------------------------------------------------------------------------
inline Circle::Circle(const Vector2& center, float radius) : m_center(center), m_radius(radius)
{
}

//-----------------------------------------------------------------------------
inline Circle::Circle(const Circle& circle) : m_center(circle.m_center), m_radius(circle.m_radius)
{
}

//-----------------------------------------------------------------------------
inline const Vector2& Circle::center() const
{
	return m_center;
}

//-----------------------------------------------------------------------------
inline float Circle::radius() const
{
	return m_radius;
}

//-----------------------------------------------------------------------------
inline void Circle::set_center(const Vector2& center)
{
	m_center = center;
}

//-----------------------------------------------------------------------------
inline void Circle::set_radius(float radius)
{
	m_radius = radius;
}

//-----------------------------------------------------------------------------
inline float Circle::area() const
{
	return m_radius * m_radius * math::PI;
}

} // namespace crown

