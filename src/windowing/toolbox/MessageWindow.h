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

#include "Window.h"
#include "TextBox.h"

namespace Crown
{

enum MessageWindowResult
{
	MWR_OK = 1,
	MWR_CANCEL = 2,
	MWR_YES = 4,
	MWR_NO = 8
};

class MessageWindow: public Window
{
  public:
		typedef IDelegate1<void, int> MessageResultDelegate;

    MessageWindow(WindowsManager* wm, const Str& title, const Str& message, int buttonsFlags, MessageResultDelegate* callback);

		void mButton_OnClick(Button* obj, EventArgs* args);

	virtual Str ToStr() const
	 { return "MessageWindow"; }

  private:
		Button* mOkButton;
		Button* mCancelButton;
		Button* mYesButton;
		Button* mNoButton;
		MessageResultDelegate* mCallback;
};

}

