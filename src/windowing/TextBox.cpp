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

#include "TextBox.h"
#include "Themes.h"
#include "Types.h"
#include "Device.h"
#include "GLTextRenderer.h"
#include "FontManager.h"
#include "SpecificProperties.h"
#include "Renderer.h"
#include "InputManager.h"

namespace Crown
{

TextBox::TextBox(Widget* parent, const Str& text):
	Widget(parent), mDisplayedTextIndex(0), mDisplayedTextX(0),
	mDisplayedTextOffset(0), mCaretIndex(0)
{
	AddProperty(new StrProperty("Text", &mText));

	mFont = GetFontManager()->LoadDefault();
	mTextRenderer = GetDevice()->GetRenderer()->GetTextRenderer();
	mTextRenderer->SetFont(mFont);

	SetText(text);

	SetDesiredSize(-1, 20);
}

TextBox::~TextBox()
{
}

void TextBox::SetText(const Str& value)
{
	mText = value;
	AfterTextSet();
}

void TextBox::OnSetProperty(const Str& name)
{
	if (name == "Text")
	{
		AfterTextSet();
	}
	else
	{
		Widget::OnSetProperty(name);
	}
}

void TextBox::AfterTextSet()
{
	if (mCaretIndex > mText.GetLength())
	{
		mCaretIndex = mText.GetLength();	
	}

	if (mDisplayedTextIndex > mText.GetLength())
	{
		mDisplayedTextIndex = mText.GetLength();
	}
	
	UpdateCaretScreenPosition();
	UpdateDisplayedText();
	ScrollCaretIntoView();
}

void TextBox::ScrollCaretIntoView()
{
	mTextRenderer->SetFont(mFont);
	if (mCaretCharPosition.x > mDisplayedTextX + mDisplayedTextOffset + Math::Max(0, GetSize().x - 6))
	{
		//scroll right
		mDisplayedTextX = mCaretCharPosition.x - Math::Max(0, (int)GetSize().x - 6);
		Point2 dummy;
		mDisplayedTextIndex = mTextRenderer->GetStrIndexFromDimensions(mText, 0, Point2(mDisplayedTextX - 10, 0), dummy);
		mDisplayedTextOffset = mDisplayedTextX - dummy.x;
		mDisplayedTextX = dummy.x;

		UpdateDisplayedText();
	}
	if (mCaretCharPosition.x < mDisplayedTextX + mDisplayedTextOffset)
	{
		//scroll left
		mDisplayedTextX = mCaretCharPosition.x;
		Point2 dummy;
		mDisplayedTextIndex = mTextRenderer->GetStrIndexFromDimensions(mText, 0, Point2(mDisplayedTextX - 10, 0), dummy);
		mDisplayedTextOffset = mDisplayedTextX - dummy.x;
		mDisplayedTextX = dummy.x;

		UpdateDisplayedText();
	}
}

void TextBox::UpdateDisplayedText()
{
	mTextRenderer->SetFont(mFont);
	uint mEndIndex;

	mEndIndex = mTextRenderer->GetStrIndexFromDimensions(mText, mDisplayedTextIndex, Point2((int)GetSize().x, (int)GetSize().y) + Point2(mDisplayedTextOffset, 0), mDisplayedTextDimensions);
	if (mEndIndex < mText.GetLength())
		mEndIndex++;
	mDisplayedText = mText.GetSubstring(mDisplayedTextIndex, mEndIndex);
}

void TextBox::UpdateCaretScreenPosition()
{
	mTextRenderer->SetFont(mFont);
	mTextRenderer->GetStrDimensions(mText, 0, mCaretIndex, mCaretCharPosition.x, mCaretCharPosition.y);
	mCaretCharPosition.y = 0;
}

void TextBox::MoveCaretLeft()
{
	if (mCaretIndex == 0)
		return;

	mTextRenderer->SetFont(mFont);

	Point2 charSize;
	mTextRenderer->GetStrDimensions(mText, mCaretIndex-1, mCaretIndex, charSize.x, charSize.y);
	mCaretCharPosition.x -= charSize.x;
	mCaretIndex--;
	ScrollCaretIntoView();
}

void TextBox::MoveCaretRight()
{
	if (mCaretIndex == mText.GetLength())
		return;

	mTextRenderer->SetFont(mFont);

	Point2 charSize;
	mTextRenderer->GetStrDimensions(mText, mCaretIndex, mCaretIndex+1, charSize.x, charSize.y);
	
	mCaretCharPosition.x += charSize.x;
	mCaretIndex++;
	ScrollCaretIntoView();
}

void TextBox::OnTextInput(TextInputEventArgs* args)
{
	args->StopPropagation(true);
	
	if (args->GetText() == "\b")
		return;

	Str before, after;
	if (mCaretIndex > 0)
		before = mText.GetSubstring(0, mCaretIndex);
	if (mCaretIndex < mText.GetLength())
		after = mText.GetSubstring(mCaretIndex, mText.GetLength());

	mText = before + args->GetText() + after;
	mCaretIndex += args->GetText().GetLength();

	UpdateDisplayedText();
	UpdateCaretScreenPosition();
	ScrollCaretIntoView();
}

void TextBox::OnMouseUp(MouseButtonEventArgs* args)
{
	Widget::OnMouseUp(args);
	if (args->IsClick())
	{
		args->StopPropagation(true);
		GetFocus();

		mTextRenderer->SetFont(mFont);
		
		//Find the caret position
		Point2 mousePosition = GetDevice()->GetInputManager()->GetMouse()->GetCursorXY() - GetScreenPosition();
		mousePosition += Point2(-2 + mDisplayedTextOffset, -2);
		mCaretIndex = mTextRenderer->GetStrIndexFromDimensions(mText, mDisplayedTextIndex, mousePosition, mCaretCharPosition);
		mCaretCharPosition.x += mDisplayedTextX;
		ScrollCaretIntoView();
	}
}

void TextBox::OnKeyDown(KeyboardEventArgs* args)
{
	Widget::OnKeyDown(args);
	args->StopPropagation(true);
	switch (args->GetKey())
	{
		case KC_LEFT:
			MoveCaretLeft();
			break;

		case KC_RIGHT:
			MoveCaretRight();
			break;

		case KC_BACKSPACE:
			if (mCaretIndex > 0)
			{
				mCaretIndex--;
				SetText(mText.GetSubstring(0, mCaretIndex) + mText.GetSubstring(mCaretIndex+1, mText.GetLength()));
			}
			break;
			
		default:
			break;
	}
}

void TextBox::OnSizeChanged(WindowingEventArgs* /*args*/)
{
	UpdateDisplayedText();
}

void TextBox::OnDraw(DrawingClipInfo& clipInfo)
{
	Widget::DrawInit(clipInfo);
	Themes::Draw(TF_LISTVIEW, GetSize());

	Renderer* r = GetDevice()->GetRenderer();

	mTextRenderer->SetFont(mFont);

	Point2 sPos = GetScreenPosition();
	//r->_SetScissorParams(sPos.x+1, 600 - (sPos.y + GetSize().y-2), GetSize().x-2, GetSize().y-4);
	r->SetScissorBox(clipInfo.sx + 1, clipInfo.sy + 1, clipInfo.sw - 2, clipInfo.sh - 2);

	int yy;
	yy = (int)((GetSize().y - 4 - mTextRenderer->GetMaxTextHeight()) / 2.0f);

	bool hasFocus = HasTextInputFocus();

	if (hasFocus)
	{
		mTextRenderer->SetColor(Color4(255 / 255.0f, 134 / 255.0f, 86 / 255.0f));
	}
	else
	{
		mTextRenderer->SetColor(Color4(86 / 255.0f, 134 / 255.0f, 255 / 255.0f));
	}

	mTextRenderer->Draw(mDisplayedText, 3 - mDisplayedTextOffset, 2 + yy);

	if (hasFocus)
	{
		r->DrawRectangle(mCaretCharPosition + Point2(3 - mDisplayedTextX - mDisplayedTextOffset, 2 + yy - 1),
		                 Point2(1, mTextRenderer->GetMaxTextHeight() + 2), DM_BORDER);
	}

	Widget::DrawChildren(clipInfo);
}

} //namespace Crown
