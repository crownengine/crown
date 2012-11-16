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
#include "TextureManager.h"

namespace crown
{

Font::Font() :
	mMaxTextHeight(0),
	mMaxCharacterHeight(0),
	mMaxCharacterWidth(0)
{
}

Font::~Font()
{
}

void Font::LoadFromFile(const Str& filename)
{
	mTexture = GetTextureManager()->Load(filename.c_str(), false);

	if (mTexture)
	{
		mTexture->SetFilter(TF_NEAREST);
	}

	Filesystem* filesystem = GetFilesystem();

	Stream* fontDef = filesystem->OpenStream((filename + Str(".txt")).c_str(), SOM_READ);

	if (fontDef)
	{
		TextReader textReader(fontDef);
		char buf[1024];

		while (textReader.ReadString(buf, 1024) != NULL)
		{
			int32_t ch;
			float a, b, c, d, e, f, g, h;

			sscanf(buf, "%d %f %f %f %f %f %f %f %f\n", &ch, &a, &b, &c, &d, &e, &f, &g, &h);

			SetCodeGlyphMetrics(ch, a, b, c, d, e, f, g, h);
		}

		filesystem->Close(fontDef);
	}
}

void Font::Load(const char* name)
{
	LoadFromFile(name);
}

void Font::Unload(const char* name, bool reload)
{
}

Glyph& Font::GetGlyph(uint32_t code)
{
	if (mCodeGlyphDict.Contains(code))
	{
		return mCodeGlyphDict[code];
	}

	static Glyph nullGlyph;
	return nullGlyph;
}

void Font::SetCodeGlyphMetrics(uint32_t code, float left, float right, float bottom, float top, float width, float height, float advance, float baseline)
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

