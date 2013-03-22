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
#include "String.h"
#include "Color4.h"
#include "Vec2.h"
#include "Font.h"
#include "Point2.h"

namespace crown
{

enum TextAlignment
{
	TA_LEFT		= 0,
	TA_CENTER	= 1,
	TA_RIGHT	= 2
};

class TextRenderer
{

public:

			TextRenderer();
			~TextRenderer();

	void	BeginDraw();
	void	EndDraw();
	//void	Draw(const char* string, int32_t x, int32_t y, Font* font);

				//! Returns the sizes in pixels of the given string, when rendered with the current font
	void		GetStrDimensions(const char* string, uint32_t start, uint32_t end, int32_t& width, int32_t& height);
				//! Returns the character index of the character at the given position
	int32_t		GetStrIndexFromDimensions(const char* string, uint32_t start, const Point2& position, Point2& charPosition);
				//! Returns the max height of the text in the current font
	int32_t		GetMaxTextHeight();


	void GetStrDimensions(const char* string, int32_t& width, int32_t& height)
	{
		//GetStrDimensions(string, 0, -1, width, height);
	}

	void GetStrDimensions(const char* string, uint32_t start, int32_t& width, int32_t& height)
	{
		//GetStrDimensions(string, start, -1, width, height);
	}

private:

	
};

} // namespace crown

