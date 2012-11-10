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

#include "SpriteAnimator.h"
#include <cstdlib>

namespace crown
{

SpriteAnimator::SpriteAnimator():
	mSprite(NULL), mFrameIndex(0), mAngle(0.0), mScaleX(1.0), mScaleY(1.0), mFrameCounter(0), mFrameInterval(0)
{
}

SpriteAnimator::~SpriteAnimator()
{
	
}

void SpriteAnimator::SetSprite(Sprite* sprite)
{
	mSprite = sprite;
}

void SpriteAnimator::SetAngle(real angle)
{
	mAngle = angle;
}

int SpriteAnimator::GetFrameIndex()
{
	return mFrameIndex;
}

void SpriteAnimator::SetFrameIndex(int index)
{
	if (mSprite)
	{
		if (index >= 0 && index < mSprite->GetFrameCount())
		{
			mFrameIndex = index;
		}
	}
}

void SpriteAnimator::SetFrameInterval(int interval)
{
	mFrameInterval = interval;
	if (mFrameCounter == 0 || mFrameCounter > interval)
	{
		mFrameCounter = interval;
	}
}

void SpriteAnimator::SetScaleX(real scaleX)
{
	mScaleX = scaleX;
}

void SpriteAnimator::SetScaleY(real scaleY)
{
	mScaleY = scaleY;
}

void SpriteAnimator::RandomizeFrameStart()
{
	mFrameIndex = rand() % mSprite->GetFrameCount();
	mFrameCounter = rand() % mFrameInterval;
}

void SpriteAnimator::Render()
{
	if (mSprite)
	{
		if (mFrameCounter > 0)
		{
			mFrameCounter --;
			if (mFrameCounter == 0)
			{
				mFrameCounter = mFrameInterval;
				mFrameIndex = (mFrameIndex + 1) % mSprite->GetFrameCount();
			}
		}
		mSprite->draw(mFrameIndex, mAngle, mScaleX, mScaleY);
	}
}

} //namespace crown
