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
#include "Texture.h"
#include "Rect.h"

namespace Crown
{

//! A frame of a Sprite, that binds to a rectangle in a texture, with a certain offset
class Frame
{
public:
	Frame();
	Frame(Texture* tex, uint tx1, uint ty1, uint tx2, uint ty2);
	~Frame();

	inline void Set(Texture* tex)
	{
		Set(tex, 0, 0, tex->GetWidth(), tex->GetHeight());
	}

	void Set(Texture* tex, uint tx1, uint ty1, uint tx2, uint ty2);

	inline void SetOffset(float x, float y)
	{
		mOffset.x = x;
		mOffset.y = y;
	}

	inline const Vec2& GetSize() const
	{
		return mSize;
	}

private:
	Texture* mTexture;
	Rect mTextureRect;
	Vec2 mOffset;
	Vec2 mSize;

	friend class Sprite;
};

} //namespace Crown

