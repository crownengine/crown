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
#include "Str.h"
#include "TextRenderer.h"
#include "Font.h"

namespace Crown
{

class TextBox: public Widget
{
public:
	TextBox(Widget* parent, const Str& text = "");
	virtual ~TextBox();

	void SetText(const Str& value);
	inline const Str& GetText() const
	{
		return mText;
	}
	virtual void OnDraw(DrawingClipInfo& clipInfo);

	virtual void OnSetProperty(const Str& name);

	virtual Str ToStr() const
	 { return "TextBox"; }

protected:
	virtual void OnTextInput(TextInputEventArgs* args);
	virtual void OnMouseUp(MouseButtonEventArgs* args);
	virtual void OnKeyDown(KeyboardEventArgs* args);
	virtual void OnSizeChanged(WindowingEventArgs* args);

private:
	Str mText;
	Str mDisplayedText;
	uint32_t mDisplayedTextIndex; //< Index in mText of the first character of the displayed text

	int mDisplayedTextX;
	Point2 mDisplayedTextDimensions;
	int mDisplayedTextOffset;

	
	uint32_t mCaretIndex;
	Point2 mCaretCharPosition;

	TextRenderer* mTextRenderer;
	Font* mFont;

	void AfterTextSet();

	void ScrollCaretIntoView();
	void UpdateDisplayedText();
	void UpdateCaretScreenPosition();
	void MoveCaretLeft();
	void MoveCaretRight();
};

} //namespace Crown
