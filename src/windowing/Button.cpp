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

#include "Button.h"
#include <GL/glew.h> // FIXME
#include "Types.h"
#include "Themes.h"
#include "Label.h"
#include "Window.h"
#include "WindowsManager.h"
#include "WindowContext.h"
#include "Auto.h"
#include "XWMLReader.h"
#include "SpecificProperties.h"

namespace Crown
{

Button::Button(Widget* parent):
	Widget(parent), mIsMouseDown(false)
{
	AddProperty(new ActionProperty<Button, EventArgs*>("OnClick", &OnClickEvent, this));
}

Button::Button(Widget* parent, const Str& caption):
	Widget(parent), mIsMouseDown(false)
{
	SetDesiredSize(-1, 20);
	Label* lbl = new Crown::Label(this);
	lbl->SetText(caption);
	lbl->SetHorizontalAlign(LHA_CENTER);
}

Button::~Button()
{
}

void Button::OnMouseDown(MouseButtonEventArgs* /*args*/)
{
	MouseCapture();
	mIsMouseDown = true;
}

void Button::OnMouseUp(MouseButtonEventArgs* args)
{
	MouseRelease();
	mIsMouseDown = false;

	if (args->IsClick())
	{
		OnClickEvent.Fire(this, NULL);
	}
}

void Button::OnDraw(DrawingClipInfo& clipInfo)
{
	if (mIsMouseDown && IsMouseOver())
	{
		Themes::Draw(TF_BUTTON_PRESSED, GetSize());
	}
	else
	{
		if (IsMouseOver())
		{
			Themes::Draw(TF_BUTTON_HIGHLIGHTED, GetSize());
		}
		else
		{
			Themes::Draw(TF_BUTTON_NORMAL, GetSize());
		}
	}

	//TODO: Use Widget::OnDraw() to draw children, and set a translation to achieve the pressed effect
	if (mDrawDebugBorder)
	{
		DebugDrawBorder();
	}

	if (mIsMouseDown && IsMouseOver())
	{
		glTranslatef(-1.0f, -1.0f, 0.0f);
	}

	DrawChildren(clipInfo);
}

} //namespace Crown
