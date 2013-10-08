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

namespace crown
{

class Circle;

/// 2D rectangle.
///
/// Used mainly for collision detection and intersection tests.
class Rect
{
public:

	/// Does nothing for efficiency.
					Rect();		

	/// Constructs from  @a min and @a max
					Rect(const Vector2& min, const Vector2& max);		
					Rect(const Rect& rect);	

	const Vector2&		min() const;					
	const Vector2&		max() const;					
	void			set_min(const Vector2& min);				
	void			set_max(const Vector2& max);			

	Vector2			center() const;					
	float			radius() const;					
	float			area() const;		

	/// Returns the diagonal of the rect.
	Vector2			size() const;						

	/// Returns whether @a point point is contained into the rect.
	bool			contains_point(const Vector2& point) const;

	/// Returns whether the rect intersects @a r.
	bool			intersects_rect(const Rect& rect) const;	

	/// Sets the Rect from a @a center and a @a width - @a height
	void			set_from_center_and_dimensions(Vector2 center, float width, float height);	

	/// Returns the four vertices of the rect.
	void			vertices(Vector2 v[4]) const;

	/// Returns the @a index -th vetex of the rect.
	Vector2			vertex(uint32_t index) const;			

	/// Returns the equivalent circle.
	Circle			to_circle() const;

	/// Ensures that min and max aren't swapped.
	void			fix();									

private:

	Vector2			m_min;
	Vector2			m_max;
};

//-----------------------------------------------------------------------------
inline Rect::Rect()
{
}

//-----------------------------------------------------------------------------
inline Rect::Rect(const Vector2& min, const Vector2& max) : m_min(min), m_max(max)
{
}

//-----------------------------------------------------------------------------
inline Rect::Rect(const Rect& rect) : m_min(rect.m_min), m_max(rect.m_max)
{
}

//-----------------------------------------------------------------------------
inline const Vector2& Rect::min() const
{
	return m_min;
}

//-----------------------------------------------------------------------------
inline const Vector2& Rect::max() const
{
	return m_max;
}

//-----------------------------------------------------------------------------
inline void Rect::set_min(const Vector2& min)
{
	m_min = min;
}

//-----------------------------------------------------------------------------
inline void Rect::set_max(const Vector2& max)
{
	m_max = max;
}

} // namespace crown

