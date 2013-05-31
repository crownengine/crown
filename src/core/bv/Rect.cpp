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

#include "Assert.h"
#include "Rect.h"
#include "Circle.h"
#include "MathUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
bool Rect::contains_point(const Vec2& point) const
{
	return (point.x >= m_min.x && point.y >= m_min.y &&
			point.x <= m_max.x && point.y <= m_max.y);
}

//-----------------------------------------------------------------------------
bool Rect::intersects_rect(const Rect& rect) const
{
	if (rect.m_min.x > m_max.x || rect.m_max.x < m_min.x || rect.m_min.y > m_max.y || rect.m_max.y < m_min.y)
		return false;
	return true;
}

//-----------------------------------------------------------------------------
void Rect::set_from_center_and_dimensions(Vec2 center, real width, real height)
{
	m_min.x = (real)(center.x - width  / 2.0);
	m_min.y = (real)(center.y - height / 2.0);
	m_max.x = (real)(center.x + width  / 2.0);
	m_max.y = (real)(center.y + height / 2.0);
}

//-----------------------------------------------------------------------------
void Rect::vertices(Vec2 v[4]) const
{
	// 3 ---- 2
	// |      |
	// |      |
	// 0 ---- 1
	v[0].x = m_min.x;
	v[0].y = m_min.y;
	v[1].x = m_max.x;
	v[1].y = m_min.y;
	v[2].x = m_max.x;
	v[2].y = m_max.y;
	v[3].x = m_min.x;
	v[3].y = m_max.y;
}

//-----------------------------------------------------------------------------
Vec2 Rect::vertex(uint32_t index) const
{
	CE_ASSERT(index < 4, "Index must be < 4");

	switch (index)
	{
		case 0:
			return Vec2(m_min.x, m_min.y);
		case 1:
			return Vec2(m_max.x, m_min.y);
		case 2:
			return Vec2(m_max.x, m_max.y);
		case 3:
			return Vec2(m_min.x, m_max.y);
	}

	return Vec2::ZERO;
}

//-----------------------------------------------------------------------------
Vec2 Rect::center() const
{
	return (m_min + m_max) * 0.5;
}

//-----------------------------------------------------------------------------
real Rect::radius() const
{
	return (m_max - (m_min + m_max) * 0.5).length();
}

//-----------------------------------------------------------------------------
real Rect::area() const
{
	return (m_max.x - m_min.x) * (m_max.y - m_min.y);
}

//-----------------------------------------------------------------------------
Vec2 Rect::size() const
{
	return (m_max - m_min);
}

//-----------------------------------------------------------------------------
void Rect::fix()
{
	if (m_min.x > m_max.x)
	{
		math::swap(m_min.x, m_max.x);
	}

	if (m_min.y > m_max.y)
	{
		math::swap(m_min.y, m_max.y);
	}
}

//-----------------------------------------------------------------------------
Circle Rect::to_circle() const
{
	return Circle(center(), radius());
}

} // namespace crown

