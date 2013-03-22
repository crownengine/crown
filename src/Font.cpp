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

#include "Font.h"
#include "MathUtils.h"
#include "Types.h"
#include "Image.h"
#include "Log.h"
#include "Filesystem.h"

namespace crown
{

FontResource::FontResource() :
	mMaxTextHeight(0),
	mMaxCharacterHeight(0),
	mMaxCharacterWidth(0)
{
}

FontResource::~FontResource()
{
}

Glyph& FontResource::GetGlyph(uint32_t code)
{
	if (mCodeGlyphDict.Contains(code))
	{
		return mCodeGlyphDict[code];
	}

	static Glyph nullGlyph;
	return nullGlyph;
}

void FontResource::SetCodeGlyphMetrics(uint32_t code, float left, float right, float bottom, float top, float width, float height, float advance, float baseline)
{
	if (mCodeGlyphDict.Contains(code))
	{
		mCodeGlyphDict[code].SetMetrics(left, right, bottom, top, width, height, advance, baseline);
	}
	else
	{
		mCodeGlyphDict[code] = Glyph(code, left, right, bottom, top, width, height, advance, baseline);
	}
}

} // namespace crown

