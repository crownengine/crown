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

#include "ScrollArea.h"
#include "StackLayout.h"
#include "Button.h"
#include "CanvasLayout.h"

namespace Crown
{

ScrollArea::ScrollArea(Widget* parent):
	Widget(parent), mScrollX(false), mScrollY(true)
{
	StackLayout* sl = new StackLayout(this);
	sl->SetOrientation(SO_HORIZONTAL);

	mContainer = new Widget(sl);
	mChildrenContainer = new Widget(mContainer);
	mChildrenContainer->SetMargins(0, 0, -1, -1);
	mChildrenContainer->SetFitToChildren((FitToChildrenEnum)((mScrollX ? FTC_HORIZONTALLY : FTC_NONE) | 
																													 (mScrollY ? FTC_VERTICALLY : FTC_NONE)));

	StackLayout* verticalBarSL = new StackLayout(sl);
	verticalBarSL->SetDesiredSize(16, -1);

	Button* btnScrollUp = new Button(verticalBarSL, "^");
	Button* btnScrollDown = new Button(verticalBarSL, "v");
	btnScrollUp->SetDesiredSize(-1, -1);
	btnScrollDown->SetDesiredSize(-1, -1);

	btnScrollUp->OnClickEvent += CreateDelegate(this, &ScrollArea::btnScrollUp_OnClick);
	btnScrollDown->OnClickEvent += CreateDelegate(this, &ScrollArea::btnScrollDown_OnClick);
}

ScrollArea::~ScrollArea()
{

}

Widget* ScrollArea::GetContentWidget()
{
  return mChildrenContainer;
}

void ScrollArea::btnScrollUp_OnClick(Button* /*obj*/, Crown::EventArgs* /*args*/)
{
	Point32_t2 pos = mChildrenContainer->GetTranslation();
	if (pos.y < 0)
		mChildrenContainer->SetTranslation(pos.x, Math::Min(0, pos.y + 10));
}

void ScrollArea::btnScrollDown_OnClick(Button* /*obj*/, Crown::EventArgs* /*args*/)
{
	Point32_t2 pos = mChildrenContainer->GetTranslation();
	int32_t yval = -mChildrenContainer->GetSize().y + mContainer->GetSize().y;
	if (pos.y > yval)
		mChildrenContainer->SetTranslation(pos.x, Math::Max(yval, pos.y - 10));
}

}