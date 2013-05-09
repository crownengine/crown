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

namespace crown
{

class Glyph
{
public:

	/// Constructor
	Glyph() :
		m_code_point(0),
		m_left(0),
		m_right(0),
		m_bottom(0),
		m_top(0),
		m_width(0),
		m_height(0),
		m_advance(0),
		m_baseline(0)
	{
	}

	/// Constructor
	Glyph(uint32_t code, float left, float right, float bottom, float top, float width, float height, float advance, float baseline) :
		m_code_point(code),
		m_left(left),
		m_right(right),
		m_bottom(bottom),
		m_top(top),
		m_width(width),
		m_height(height),
		m_advance(advance),
		m_baseline(baseline)
	{
	}

	/// Destructor
	~Glyph()
	{
	}

	/// Returns the glyph's metrics
	void metrics(float& left, float& right, float& bottom, float& top, float& width, float& height, float& advance, float& baseline) const
	{
		left = m_left;
		right = m_right;
		bottom = m_bottom;
		top = m_top;
		width = m_width;
		height = m_height;
		advance = m_advance;
		baseline = m_baseline;
	}

	/// Sets the glyph's metrics
	void set_metrics(float left, float right, float bottom, float top, float width, float height, float advance, float baseline)
	{
		m_left = left;
		m_right = right;
		m_bottom = bottom;
		m_top = top;
		m_width = width;
		m_height = height;
		m_advance = advance;
		m_baseline = baseline;
	}

	/// Returns the glyph's code point
	uint32_t code_point() const
	{
		return m_code_point;
	}

private:

	uint32_t m_code_point;
	float m_left;
	float m_right;
	float m_bottom;
	float m_top;
	float m_width;
	float m_height;
	float m_advance;
	float m_baseline;
};

} // namespace crown

