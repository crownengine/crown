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

#include "Label.h"
#include "Themes.h"
#include "Types.h"
#include "Device.h"
#include "TextRenderer.h"
#include "FontManager.h"
#include "SpecificProperties.h"
#include "Renderer.h"

namespace Crown
{

Label::Label(Widget* parent, Str text):
	Widget(parent), mHorizontalAlign(LHA_LEFT), mVerticalAlign(LVA_CENTER)
{
	EnumProperty* prop = new EnumProperty("HorizontalTextAlign", (int32_t*)&mHorizontalAlign);
	prop->AddValueMapping("Left", LHA_LEFT);
	prop->AddValueMapping("Center", LHA_CENTER);
	prop->AddValueMapping("Right", LHA_RIGHT);
	AddProperty(prop);

	prop = new EnumProperty("VerticalTextAlign", (int32_t*)&mVerticalAlign);
	prop->AddValueMapping("Top", LVA_TOP);
	prop->AddValueMapping("Center", LVA_CENTER);
	prop->AddValueMapping("Bottom", LVA_BOTTOM);
	AddProperty(prop);

	AddProperty(new StrProperty("Text", &mText));

	SetDesiredSize(-1, -1);
	mFont = GetFontManager()->LoadDefault();
	SetText(text);
}

Label::~Label()
{

}

void Label::OnSetProperty(const Str& name)
{
	if (name == "Text")
	{
		CalculateTextDimensions();
	}
	else
	{
		Widget::OnSetProperty(name);
	}
}

void Label::SetText(Str value)
{
	mText = value;
	CalculateTextDimensions();
}

void Label::CalculateTextDimensions()
{
	int32_t dimX, dimY;

	TextRenderer* textRenderer = GetDevice()->GetRenderer()->GetTextRenderer();
	textRenderer->SetFont(mFont);
	textRenderer->GetStrDimensions(mText, 0, dimX, dimY);

	dimY = textRenderer->GetMaxTextHeight();

	mTextSize.x = dimX;
	mTextSize.y = dimY;

	SetMinimumSize(dimX + 2, dimY + 2);
}

void Label::OnDraw(DrawingClipInfo& clipInfo)
{
	int32_t yy;
	int32_t xx;

	TextRenderer* textRenderer = GetDevice()->GetRenderer()->GetTextRenderer();
	textRenderer->SetFont(mFont);

	//mTextSize.y is the rendered text height, depending on mVerticalAlign place the yy
	switch(mVerticalAlign)
	{
		case LVA_TOP:
			yy = 0;
		break;
		case LVA_CENTER:
			yy = (GetSize().y - mTextSize.y) / 2;
		break;
		case LVA_BOTTOM:
			yy = GetSize().y - mTextSize.y;
		break;
	}

	//mTextSize.x is the rendered text width, depending on mHorizontalAlign place the xx
	switch(mHorizontalAlign)
	{
		case LHA_LEFT:
			xx = 0;
		break;
		case LHA_CENTER:
			xx = (GetSize().x - mTextSize.x) / 2;
		break;
		case LHA_RIGHT:
			xx = GetSize().x - mTextSize.x;
		break;
	}
	
	textRenderer->SetColor(Color4(86, 134, 255));
	textRenderer->Draw(mText, xx, yy);

	Widget::OnDraw(clipInfo);
}

} //namespace Crown
