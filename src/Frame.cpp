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

#include "Frame.h"
#include "Types.h"
#include "Sprite.h"

namespace Crown
{

Frame::Frame():
	mTexture(NULL), mOffset(0, 0)
{
}

Frame::Frame(Texture* tex, uint tx1, uint ty1, uint tx2, uint ty2):
	mOffset(0, 0)
{
	Set(tex, tx1, ty1, tx2, ty2);
}

Frame::~Frame()
{
}

void Frame::Set(Texture* tex, uint tx1, uint ty1, uint tx2, uint ty2)
{
	mTexture = tex;
	//Verify that the coordinates lie inside the texture
	uint maxW = mTexture->GetWidth();
	uint maxH = mTexture->GetHeight();

	if (tx1 > maxW)
	{
		tx1 = maxW;
	}

	if (tx2 > maxW)
	{
		tx2 = maxW;
	}

	if (ty1 > maxH)
	{
		ty1 = maxH;
	}

	if (ty2 > maxH)
	{
		ty2 = maxH;
	}

	//And scale them into the [0,1] space.
	mTextureRect.min.x = tx1 / (float)mTexture->GetWidth();
	mTextureRect.min.y = ty1 / (float)mTexture->GetHeight();
	mTextureRect.max.x = tx2 / (float)mTexture->GetWidth();
	mTextureRect.max.y = ty2 / (float)mTexture->GetHeight();
	mSize.x = (float)(tx2 - tx1);
	mSize.y = (float)(ty2 - ty1);
}

} //namespace Crown
