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
#include "Widget.h"
#include "Themes.h"

namespace Crown
{

class ThemeSpriteWidget: public Widget
{
public:
	ThemeSpriteWidget(Widget* p, ThemesSprites spriteId, uint32_t frameNumber):
		Widget(p), mSpriteId(spriteId), mFrameNumber(frameNumber)
	{
		Sprite* spr = Themes::GetSprite(spriteId);

		if (spr)
		{
			Frame* fr = spr->GetFrame(frameNumber);
			SetDesiredSize((int32_t)fr->GetSize().x, (int32_t)fr->GetSize().y);
		}

		SetMouseSensible(false);
	}

	virtual ~ThemeSpriteWidget()
	{
	}

	void OnDraw(DrawingClipInfo& clipInfo)
	{
		Themes::Draw(mSpriteId, mFrameNumber);

		Widget::OnDraw(clipInfo);
	}

private:
	ThemesSprites mSpriteId;
	uint32_t mFrameNumber;
};

} //namespace Crown
