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

namespace Crown
{

class Glyph
{

public:

	//! Constructor
	Glyph() :
		mCodePoint(0),
		mLeft(0),
		mRight(0),
		mBottom(0),
		mTop(0),
		mWidth(0),
		mHeight(0),
		mAdvance(0),
		mBaseline(0)
	{
	}

	//! Constructor
	Glyph(wchar_t code, float left, float right, float bottom, float top, float width, float height, float advance, float baseline) :
		mCodePoint(code),
		mLeft(left),
		mRight(right),
		mBottom(bottom),
		mTop(top),
		mWidth(width),
		mHeight(height),
		mAdvance(advance),
		mBaseline(baseline)
	{
	}

	//! Destructor
	~Glyph()
	{
	}

	//! Returns the glyph's metrics
	void GetMetrics(float& left, float& right, float& bottom, float& top, float& width, float& height, float& advance, float& baseline) const
	{
		left = mLeft;
		right = mRight;
		bottom = mBottom;
		top = mTop;
		width = mWidth;
		height = mHeight;
		advance = mAdvance;
		baseline = mBaseline;
	}

	//! Sets the glyph's metrics
	void SetMetrics(float left, float right, float bottom, float top, float width, float height, float advance, float baseline)
	{
		mLeft = left;
		mRight = right;
		mBottom = bottom;
		mTop = top;
		mWidth = width;
		mHeight = height;
		mAdvance = advance;
		mBaseline = baseline;
	}

	//! Returns the glyph's code point
	wchar_t GetCodePoint() const
	{
		return mCodePoint;
	}

private:

	wchar_t mCodePoint;
	float mLeft;
	float mRight;
	float mBottom;
	float mTop;
	float mWidth;
	float mHeight;
	float mAdvance;
	float mBaseline;
};

} // namespace Crown

