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
#include "List.h"
#include "Dictionary.h"
#include "Glyph.h"
#include "Resource.h"
#include "Image.h"

namespace Crown
{

class Image;
class Texture;

/**
	Font resource for using in text rendering.
*/
class Font : public Resource
{

	typedef Dictionary<uint, Glyph> CodeGlyphDict;

public:

							Font();	//! Constructor
							~Font();	//! Destructor

	virtual void			Load(const char* name);
	virtual void			Unload(const char* name, bool reload);

	void					LoadFromFile(const Str& filename);

	Glyph&					GetGlyph(uint code);	//! Returns the glyph for the desired point code
	void					SetCodeGlyphMetrics(uint code, float left, float right, float bottom, float top, float width, float height, float advance, float baseline);

	inline uint				_GetMaxTextHeight() { return mMaxTextHeight; }
	inline uint				_GetMaxCharacterHeight() { return mMaxCharacterHeight; }
	inline uint				_GetMaxCharacterWidth() { return mMaxCharacterWidth; }

	Texture*				GetTexture() { return mTexture; }

private:

	CodeGlyphDict			mCodeGlyphDict;

	uint					mMaxTextHeight;
	uint					mMaxCharacterHeight;
	uint					mMaxCharacterWidth;

	Texture*				mTexture;
};

} // namespace Crown

