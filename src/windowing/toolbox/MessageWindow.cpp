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

#include "MessageWindow.h"
#include "StackLayout.h"

namespace Crown
{

MessageWindow::MessageWindow(WindowsManager* wm, const Str& title, const Str& message, int32_t buttonsFlags, MessageResultDelegate* callback):
	Window(wm, 300, 300, 250, 70, title), mCallback(callback)
{


	StackLayout* layout = new StackLayout(GetContentWidget());
	layout->SetMargins(2, 10, 2, 2);

	Label* lbl;
	lbl = new Label(layout, message);

	StackLayout* buttonLayout = new StackLayout(layout);
	buttonLayout->SetMargins(0, 2, 0, 0);
	buttonLayout->SetOrientation(SO_HORIZONTAL);

	//If no valid flag has been specified, use MWR_OK
	if ((buttonsFlags & (MWR_OK | MWR_CANCEL | MWR_YES | MWR_NO)) == 0)
		buttonsFlags = MWR_OK;

	//Note: do not use the same instance of delegate for all the events, because the += operation transfers the ownership
	if (buttonsFlags & MWR_OK)
	{
		mOkButton = new Button(buttonLayout, "Ok");
		mOkButton->SetMargins(2, 0, 2, 0);
		mOkButton->OnClickEvent += CreateDelegate(this, &MessageWindow::mButton_OnClick);
	}
	if (buttonsFlags & MWR_CANCEL)
	{
		mCancelButton = new Button(buttonLayout, "Cancel");
		mCancelButton->SetMargins(2, 0, 2, 0);
		mCancelButton->OnClickEvent += CreateDelegate(this, &MessageWindow::mButton_OnClick);
	}
	if (buttonsFlags & MWR_YES)
	{
		mYesButton = new Button(buttonLayout, "Yes");
		mYesButton->SetMargins(2, 0, 2, 0);
		mYesButton->OnClickEvent += CreateDelegate(this, &MessageWindow::mButton_OnClick);
	}
	if (buttonsFlags & MWR_NO)
	{
		mNoButton = new Button(buttonLayout, "No");
		mNoButton->SetMargins(2, 0, 2, 0);
		mNoButton->OnClickEvent += CreateDelegate(this, &MessageWindow::mButton_OnClick);
	}
}

void MessageWindow::mButton_OnClick(Button* button, EventArgs* /*args*/)
{
	if (mCallback)
	{
		int32_t resultFlags = 0;

		if (button == mOkButton)
			resultFlags |= MWR_OK;
		if (button == mCancelButton)
			resultFlags |= MWR_CANCEL;
		if (button == mYesButton)
			resultFlags |= MWR_YES;
		if (button == mNoButton)
			resultFlags |= MWR_NO;

		mCallback->Invoke(resultFlags);
		mCallback->Trash();
	}
	Close();
}

}
