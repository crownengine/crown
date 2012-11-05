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

#include "Rect.h"
#include "Circle.h"

namespace crown
{

//-----------------------------------------------------------------------------
bool Rect::contains_point(const Vec2& point) const
{
	return (point.x >= min.x && point.y >= min.y &&
			point.x <= max.x && point.y <= max.y);
}

//-----------------------------------------------------------------------------
bool Rect::intersects_rect(const Rect& Rect) const
{
	//return (contains_point(Rect.min) || contains_point(Rect.max));
	if (Rect.min.x > max.x || Rect.max.x < min.x || Rect.min.y > max.y || Rect.max.y < min.y)
		return false;
	return true;
}

//-----------------------------------------------------------------------------
void Rect::set_from_center_and_dimensions(Vec2 center, real width, real height)
{
	min.x = (real)(center.x - width  / 2.0);
	min.y = (real)(center.y - height / 2.0);
	max.x = (real)(center.x + width  / 2.0);
	max.y = (real)(center.y + height / 2.0);
}

//-----------------------------------------------------------------------------
void Rect::get_vertices(Vec2 v[4]) const
{
	// 3 ---- 2
	// |      |
	// |      |
	// 0 ---- 1
	v[0].x = min.x;
	v[0].y = min.y;
	v[1].x = max.x;
	v[1].y = min.y;
	v[2].x = max.x;
	v[2].y = max.y;
	v[3].x = min.x;
	v[3].y = max.y;
}

//-----------------------------------------------------------------------------
Vec2 Rect::get_vertex(uint index) const
{
	assert(index < 4);

	switch (index)
	{
		case 0:
			return Vec2(min.x, min.y);
		case 1:
			return Vec2(max.x, min.y);
		case 2:
			return Vec2(max.x, max.y);
		case 3:
			return Vec2(min.x, max.y);
	}

	return Vec2::ZERO;
}

//-----------------------------------------------------------------------------
Vec2 Rect::get_center() const
{
	return (min + max) * 0.5;
}

//-----------------------------------------------------------------------------
real Rect::get_radius() const
{
	return (max - (min + max) * 0.5).length();
}

//-----------------------------------------------------------------------------
real Rect::get_area() const
{
	return (max.x - min.x) * (max.y - min.y);
}

//-----------------------------------------------------------------------------
Vec2 Rect::get_size() const
{
	return (max - min);
}

//-----------------------------------------------------------------------------
void Rect::fix()
{
	if (min.x > max.x)
	{
		real tmp = min.x;
		min.x = max.x;
		max.x = tmp;
	}

	if (min.y > max.y)
	{
		real tmp = min.y;
		min.y = max.y;
		max.y = tmp;
	}
}

//-----------------------------------------------------------------------------
Circle Rect::to_circle() const
{
	return Circle(get_center(), get_radius());
}

} // namespace crown

