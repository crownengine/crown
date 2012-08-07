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

#include "WindowsManager.h"
#include "Themes.h"
#include "Types.h"
#include "ThemeSpriteWidget.h"
#include "StackLayout.h"
#include "WindowContext.h"
#include "Property.h"
#include "Renderer.h"

namespace Crown
{

Point2 Window::mDefaultPosition(30, 30);

Window::Window(WindowsManager* wm, int x, int y, int width, int height, Str title):
	Widget(NULL),
	mWindowsManager(wm), mModalParent(NULL),
	mClientArea(0), mCloseButton(0), mNeedsLayout(true),
	mIsDoingModal(false), mWindowContext(NULL)
{
	InitWindow(x, y, width, height, title);
}

Window::Window(WindowsManager* wm, int width, int height, Str title):
	Widget(NULL),
	mWindowsManager(wm), mModalParent(NULL),
	mClientArea(0), mCloseButton(0), mNeedsLayout(true),
	mIsDoingModal(false), mWindowContext(NULL)
{
	InitWindow(mDefaultPosition.x, mDefaultPosition.y, width, height, title);
	//Compute the new default position
	mDefaultPosition.x += 20;
	mDefaultPosition.y += 20;
}

Window::~Window()
{
	delete mWindowContext;
}

void Window::InitWindow(int x, int y, int width, int height, Str title)
{
	mWindowsManager->mWindows.Append(this);
	SetDesiredPosition(x - 1, y - 11);
	SetDesiredSize(width + 2, height + 15);

	StackLayout* layout = new StackLayout(this);

	mDragArea = new DragArea(layout);
	mDragArea->SetDesiredSize(-1, 14);
	mDragArea->SetWidgetToDrag(this);

	StackLayout* titleLayout = new StackLayout(mDragArea);
	titleLayout->SetOrientation(SO_HORIZONTAL);

	mTitle = new Label(titleLayout, title);
	mTitle->SetMargins(1, -1, -1, -1);
	//mTitle->SetDesiredSize(-1, -1);

	mCloseButton = new Button(titleLayout);
	mCloseButton->SetDesiredSize(8, 8);
  mCloseButton->SetMargins(2, -1, 2, -1);
	ThemeSpriteWidget* tsw = new ThemeSpriteWidget(mCloseButton, TS_CLOSEBUTTON_X, 0);
  tsw->SetMargins(3, 3, -1, -1);

	mClientArea = new Widget(layout);
	mClientArea->SetMargins(1, 1, 1, 1);

	mCloseButton->OnClickEvent += CreateDelegate(this, &Window::CloseButton_OnClick);

	AddProperty(new PropertyPipe("Title", mTitle->GetProperty("Text")));
}

Widget* Window::GetContentWidget()
{
	if (mClientArea)
		return mClientArea;
	return this;
}

void Window::CloseButton_OnClick(Button* /*widget*/, EventArgs* /*args*/)
{
	Close();
}

void Window::_PerformLayout()
{
	if (mNeedsLayout)
	{
		mNeedsLayout = false;
		bool fitX = Math::TestBitmask(GetFitToChildren(), FTC_HORIZONTALLY);
		bool fitY = Math::TestBitmask(GetFitToChildren(), FTC_VERTICALLY);
		OnMeasure(fitX, fitY);
		OnArrange(GetDesiredPosition(), GetDesiredSize());
	}
}

void Window::OnDraw(DrawingClipInfo& clipInfo)
{
	Widget::DrawInit(clipInfo);
	Themes::Draw(TF_WINDOW, GetSize());
	Renderer* r = GetDevice()->GetRenderer();
	Widget::DrawChildren(clipInfo);
	
	if (mIsDoingModal)
	{
		Color4 c(0.0f, 0.0f, 0.0f, 0.1f);
		r->DrawRectangle(Point2::ZERO, GetSize(), DM_FILL, c, c);
	}
}

void Window::NotifyNeedsLayout()
{
	mNeedsLayout = true;
}

void Window::DoModal(Window* childWindow)
{
	if (!childWindow->mModalParent)
	{
		childWindow->mModalParent = this;
		mIsDoingModal = true;
		SetMouseSensible(false);
	}
}

void Window::Close()
{
	WindowCloseEventArgs ea(this);
	OnCloseEvent.Fire(this, &ea);

	if (ea.cancel)
	{
		return;
	}

	if (mModalParent)
	{
		//If a modal parent has been set, reactivate it
		mModalParent->SetMouseSensible(true);
		mModalParent->mIsDoingModal = false;
	}
	else
	{
		//Call EndModal on the windows manager, if this window was a global modal window, it will reactivate the interactions
		mWindowsManager->EndModal(this);
	}

	mWindowsManager->mWindows.Remove(mWindowsManager->mWindows.Find(this));
	Destroy();
}

} //namespace Crown
