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

#include "TextInputWindow.h"
#include "StackLayout.h"

namespace Crown
{

TextInputWindow::TextInputWindow(WindowsManager* wm, const Str& title, const Str& message, const Str& defaultText, IDelegate2<void, bool, Str>* callback):
	Window(wm, 300, 300, 250, 70, title), mCallback(callback)
{
	StackLayout* layout = new StackLayout(GetContentWidget());
	layout->SetMargins(2, 2, 2, 2);

	Label* lbl;

	lbl = new Label(layout, message);

	mTextBox = new TextBox(layout, defaultText);

	StackLayout* buttonLayout = new StackLayout(layout);
	buttonLayout->SetMargins(0, 2, 0, 0);
	buttonLayout->SetOrientation(SO_HORIZONTAL);

	mOkButton = new Button(buttonLayout, "Ok");

	mCancelButton = new Button(buttonLayout, "Cancel");
	mCancelButton->SetMargins(2, 0, 0, 0);

	mOkButton->OnClickEvent += CreateDelegate(this, &TextInputWindow::mOkButton_OnClick);
	mCancelButton->OnClickEvent += CreateDelegate(this, &TextInputWindow::mCancelButton_OnClick);
}

void TextInputWindow::mOkButton_OnClick(Button* /*obj*/, EventArgs* /*args*/)
{
	if (mCallback)
	{
		Str text = mTextBox->GetText();
		mCallback->Invoke(false, text);
		mCallback->Trash();
	}
	Close();
}

void TextInputWindow::mCancelButton_OnClick(Button* /*obj*/, EventArgs* /*args*/)
{
	if (mCallback)
	{
		mCallback->Invoke(true, "");
		mCallback->Trash();
	}
	Close();
}

}
