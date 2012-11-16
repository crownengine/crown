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
#include "Device.h"
#include <GL/glew.h> // FIXME
#include "Types.h"
#include "Themes.h"
#include "LunaTheme.h"
#include "Log.h"
#include "Scene.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "InputManager.h"

namespace Crown
{

WindowsManager::WindowsManager(Scene* creator):
	SceneNode(creator, 0, Vec3(0, 0, 0), Angles(0, 0, 0), true),
	mMouseCapturedWidget(NULL), mWidgetUnderMouse(NULL), mWidgetMouseDown(NULL),
	mInputFocusWidget(NULL), mModalWindow(NULL), mMousePosition(-10000, -10000)
{
	Device* mDevice = GetDevice();
	mDevice->GetInputManager()->RegisterMouseListener(this);
	mDevice->GetInputManager()->RegisterKeyboardListener(this);

	Themes::LoadTheme(new LunaTheme());

	//Register the default actions
	RegisterAction("QuitApplication", CreateDelegate(this, &WindowsManager::QuitApplicationAction));
	RegisterAction("CopyProperty", CreateDelegate(this, &WindowsManager::CopyProperty));

	if (creator)
		creator->GetRootSceneNode()->AddChild(this);
}

WindowsManager::~WindowsManager()
{
	for (int32_t i=0; i<mWindows.GetSize(); i++)
	{
		delete mWindows[i];
	}

	ActionDictionary::Enumerator e = mRegisteredActions.getBegin();
	while (e.next())
	{
		delete e.current().value;
	}
}

bool WindowsManager::DoModalWindow(Window* window)
{
	if (mModalWindow != NULL)
		return false;

	mModalWindow = window;

	//Perform a layout of the window to obtain actual size and place it at the center of the screen
	RenderWindow* renderWindow = GetDevice()->GetMainWindow();
	window->_PerformLayout();

	uint32_t rwWidth, rwHeight;
	renderWindow->GetMetrics(rwWidth, rwHeight);
	window->SetDesiredPosition((int32_t)Math::Floor(((float)rwWidth  - window->GetSize().x)/2.0f),
														 (int32_t)Math::Floor(((float)rwHeight - window->GetSize().y)/2.0f));

	return true;
}

void WindowsManager::EndModal(Window* window)
{
	if (mModalWindow == window)
	{
		mModalWindow = NULL;
	}
}

void WindowsManager::RegisterAction(const Str& name, ActionDelegate* slot)
{
	if (mRegisteredActions.Contains(name))
	{
		Log::E("WindowsManager already has an Action registered with name '" + name + "'");
	}
	else
	{
		mRegisteredActions[name] = slot;
	}
}

ActionDelegate* WindowsManager::GetRegisteredAction(const Str& name)
{
	if (mRegisteredActions.Contains(name))
	{
		//Return a duplicate because the caller will usually pass the delegate to an event, which transfers ownership
		return (ActionDelegate*)mRegisteredActions[name]->Duplicate();		
	}
	return NULL;
}

void WindowsManager::ButtonPressed(const MouseEvent& event)
{
	if (mWidgetUnderMouse != NULL)
	{
		mWidgetMouseDown = mWidgetUnderMouse;
		BringWindowToFront(mWidgetMouseDown->GetWindow());
		MouseDownHelper(mWidgetMouseDown);
	}
	else if (mMouseCapturedWidget != NULL)
	{
		BringWindowToFront(mMouseCapturedWidget->GetWindow());
		MouseDownHelper(mMouseCapturedWidget);
	}
}

void WindowsManager::ButtonReleased(const MouseEvent& event)
{
	if (mWidgetUnderMouse != NULL)
	{
		bool isClick = mWidgetUnderMouse == mWidgetMouseDown || (mMouseCapturedWidget != NULL && mWidgetUnderMouse->IsChildOf(mMouseCapturedWidget));
		
		MouseUpHelper(mWidgetUnderMouse, isClick);
		mWidgetMouseDown = NULL;
	}
	else if (mMouseCapturedWidget != NULL)
	{
		MouseUpHelper(mMouseCapturedWidget, false);
	}
}

void WindowsManager::CursorMoved(const MouseEvent& event)
{
	Point32_t2 newMousePosition(event.x, event.y);

	if (mMousePosition.x == -10000)
	{
		mMousePosition = newMousePosition;
	}

	//Check for the widget under mouse.
	Point32_t2 point32_t(newMousePosition);
	Widget* wdg = NULL;
	Widget* wdgToMove = NULL;

	if (mMouseCapturedWidget != NULL)
	{
		wdgToMove = mMouseCapturedWidget;
		Widget* parent = mMouseCapturedWidget->GetParent();

		if (parent != NULL)
		{
			point32_t -= parent->GetScreenPosition();
		}

		if (mMouseCapturedWidget->IsPoint32_tInside(point32_t))
		{
			wdg = mMouseCapturedWidget;
		}
	}
	else
	{
		for (int32_t i=mWindows.GetSize()-1; i>=0; i--)
		{
			Window* w = mWindows[i];

			if (w->IsMouseSensible() && w->IsPoint32_tInside(point32_t))
			{
				point32_t -= w->GetPosition() + w->GetTranslation();
				wdg = FindWidgetAt(w, point32_t);
				break;
			}
		}

		if (mModalWindow != NULL && wdg != NULL && wdg->GetWindow() != mModalWindow)
		{
			return;
		}
	}

	if (wdg == NULL)
	{
		//Mouse is not inside any window
		if (mWidgetUnderMouse != NULL)
		{
			MouseOutHelper(mWidgetUnderMouse);
			mWidgetUnderMouse = NULL;
		}

		mWidgetMouseDown = NULL;
	}
	else if (mWidgetUnderMouse != wdg)
	{
		if (mWidgetUnderMouse != NULL)
		{
			MouseOutHelper(mWidgetUnderMouse);
		}

		mWidgetUnderMouse = wdg;
		MouseInHelper(mWidgetUnderMouse);
		wdgToMove = wdg;
	}

	if (wdgToMove)
	{
		point32_t -= wdgToMove->GetPosition();
		MouseMoveHelper(wdgToMove, newMousePosition - mMousePosition);
	}

	mMousePosition = newMousePosition;
}

void WindowsManager::KeyPressed(const KeyboardEvent& event)
{
	if (event.key == KC_F1)
	{
		Widget::SetDrawDebugBorder(true);
	}

	if (mInputFocusWidget != NULL)
	{
		KeyDownHelper(mInputFocusWidget, event.key);
	}
}

void WindowsManager::KeyReleased(const KeyboardEvent& event)
{
	if (event.key == KC_F1)
	{
		Widget::SetDrawDebugBorder(false);
	}

	if (mInputFocusWidget != NULL)
	{
		KeyUpHelper(mInputFocusWidget, event.key);
	}
}

void WindowsManager::TextInput(const KeyboardEvent& event)
{
	if (mInputFocusWidget != NULL)
	{
		TextInputHelper(mInputFocusWidget, event.text);
	}
}

Widget* WindowsManager::FindWidgetAt(Widget* parent, Point32_t2& point32_t) const
{
	Widget* widgetInside = parent;

	for (int32_t i=parent->mChildren.GetSize()-1; i>=0; i--)
	{
		Widget* w = parent->mChildren[i];

		if (w->IsPoint32_tInside(point32_t))
		{
			point32_t -= w->GetPosition() + w->GetTranslation();
			widgetInside = FindWidgetAt(w, point32_t);
			break;
		}
	}

	if (!widgetInside->IsMouseSensible())
	{
		return parent;
	}

	return widgetInside;
}


//Propagate the event in the widgets hierarchy. delegate contains the point32_ter to function and the
//parameters that need to be passed to it.
void WindowsManager::LaunchEvent(Widget* targetWidget, IDelegate1<void, bool>* onEventHelper, WindowingEventArgs* args)
{
	if (!onEventHelper || !targetWidget || !args)
		return;

	List<Widget*> ancestors;
	Widget* w = targetWidget;
	while (w != NULL)
	{
		ancestors.Append(w);
		w = w->GetParent();
	}

	//Tunneling
	args->StopPropagation(false);
	for(int32_t i = ancestors.GetSize()-1; i >= 0; i--)
	{
		onEventHelper->SetCalledObject(ancestors[i]);
		onEventHelper->Invoke(true);
		if (args->IsPropagationStopped())
			break;
	}

	//Bubbling
	args->StopPropagation(false);
	for(int32_t i = 0; i < ancestors.GetSize(); i++)
	{
		onEventHelper->SetCalledObject(ancestors[i]);
		onEventHelper->Invoke(false);
		if (args->IsPropagationStopped())
			break;
	}
}

/*
 *	Widget event helpers
 */

void WindowsManager::MouseInHelper(Widget* targetWidget)
{
	MouseEventArgs args(targetWidget);
	Delegate2<Widget, void, bool, MouseEventArgs*> d(NULL, &Widget::OnMouseInHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::MouseOutHelper(Widget* targetWidget)
{
	MouseEventArgs args(targetWidget);
	Delegate2<Widget, void, bool, MouseEventArgs*> d(NULL, &Widget::OnMouseOutHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::MouseMoveHelper(Widget* targetWidget, const Point32_t2& delta)
{
	MouseMoveEventArgs args(targetWidget, delta);
	Delegate2<Widget, void, bool, MouseMoveEventArgs*> d(NULL, &Widget::OnMouseMoveHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::MouseUpHelper(Widget* targetWidget, bool isClick)
{
	MouseButtonEventArgs args(targetWidget, MB_LEFT, isClick);
	Delegate2<Widget, void, bool, MouseButtonEventArgs*> d(NULL, &Widget::OnMouseUpHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::MouseDownHelper(Widget* targetWidget)
{
	MouseButtonEventArgs args(targetWidget, MB_LEFT, false);
	Delegate2<Widget, void, bool, MouseButtonEventArgs*> d(NULL, &Widget::OnMouseDownHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::TextInputHelper(Widget* targetWidget, const Str& text)
{
	TextInputEventArgs args(targetWidget, text);
	Delegate2<Widget, void, bool, TextInputEventArgs*> d(NULL, &Widget::OnTextInputHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::GotFocusHelper(Widget* targetWidget)
{
	WindowingEventArgs args(targetWidget);
	Delegate2<Widget, void, bool, WindowingEventArgs*> d(NULL, &Widget::OnGotFocusHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::LostFocusHelper(Widget* targetWidget)
{
	WindowingEventArgs args(targetWidget);
	Delegate2<Widget, void, bool, WindowingEventArgs*> d(NULL, &Widget::OnLostFocusHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::KeyDownHelper(Widget* targetWidget, Key key)
{
	KeyboardEventArgs args(targetWidget, key);
	Delegate2<Widget, void, bool, KeyboardEventArgs*> d(NULL, &Widget::OnKeyDownHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::KeyUpHelper(Widget* targetWidget, Key key)
{
	KeyboardEventArgs args(targetWidget, key);
	Delegate2<Widget, void, bool, KeyboardEventArgs*> d(NULL, &Widget::OnKeyUpHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

void WindowsManager::SizeChangedHelper(Widget* targetWidget)
{
	WindowingEventArgs args(targetWidget);
	Delegate2<Widget, void, bool, WindowingEventArgs*> d(NULL, &Widget::OnSizeChangedHelper, true, &args);
	LaunchEvent(targetWidget, &d, &args);
}

bool WindowsManager::MouseCapture(Widget* w)
{
	if (mMouseCapturedWidget != NULL || w == NULL)
	{
		return false;
	}

	if (!w->IsMouseSensible())
	{
		return false;
	}

	mMouseCapturedWidget = w;
	return true;
}

void WindowsManager::MouseRelease()
{
	mMouseCapturedWidget = NULL;
}

void WindowsManager::_SetTextInputFocusWidget(Widget* widget)
{
	if (mInputFocusWidget != NULL)
	{
		LostFocusHelper(mInputFocusWidget);
	}
	mInputFocusWidget = widget;
	GotFocusHelper(mInputFocusWidget);
}

void WindowsManager::ReleaseWidget(Widget* w)
{
	if (mMouseCapturedWidget != NULL && mMouseCapturedWidget->IsChildOf(w))
	{
		MouseRelease();
	}

	if (mWidgetUnderMouse != NULL && mWidgetUnderMouse->IsChildOf(w))
	{
		mWidgetUnderMouse = NULL;
	}

	if (mWidgetMouseDown != NULL && mWidgetMouseDown->IsChildOf(w))
	{
		mWidgetMouseDown = NULL;
	}

	if (mInputFocusWidget != NULL && mInputFocusWidget->IsChildOf(w))
	{
		mInputFocusWidget = NULL;
	}
}

void WindowsManager::BringWindowToFront(Window* window)
{
	if (mWindows[mWindows.GetSize() - 1] != window)
	{
		mWindows.Remove(mWindows.Find(window));
		mWindows.Append(window);
	}
}

void WindowsManager::OnRegisterForRendering()
{
	if (mCreator)
		mCreator->RegisterNodeForRendering(this, RP_GUI);
}

void WindowsManager::Render()
{
	Renderer* renderer = GetDevice()->GetRenderer();
	Mat4 ortho;
	RenderWindow* rWindow = GetDevice()->GetMainWindow();
	uint32_t rwWidth, rwHeight;
	rWindow->GetMetrics(rwWidth, rwHeight);
	ortho.BuildProjectionOrtho2dRH((float)rwWidth, (float)rwHeight, -1.0f, 1.0f);
	renderer->SetMatrix(MT_PROJECTION, ortho);
	renderer->SetMatrix(MT_VIEW, Mat4::IDENTITY);
	renderer->SelectMatrix(MT_MODEL);
	renderer->_SetScissor(true);
	renderer->_SetBlending(true);
	renderer->_SetBlendingParams(BE_FUNC_ADD, BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA, Color4::BLACK);
	renderer->_SetLighting(false);

	for (int32_t i=0; i<mWindows.GetSize(); i++)
	{
		renderer->PushMatrix();
		Window* wnd = mWindows[i];
		wnd->_PerformLayout();

		if (wnd == mModalWindow)
		{
			RenderWindow* renderWindow = GetDevice()->GetMainWindow();
			//Draw a dark-gray overlay before drawing the modal window
			//TODO: Adjust this behaviour by int32_troducing windows z-ordering and foremost windows
			Color4 fillColor(0.0f, 0.0f, 0.0f, 0.3f);
			renderer->_SetScissor(false);
			renderer->DrawRectangle(Point32_t2::ZERO, Point32_t2(rwWidth, rwHeight), DM_FILL, fillColor, fillColor);
			renderer->_SetScissor(true);
		}

		Point32_t2 pos = wnd->GetPosition();
		Point32_t2 size = wnd->GetSize();
		glTranslatef((float)pos.x, (float)pos.y, 0.0f);

		DrawingClipInfo clipInfo;
		clipInfo.screenX = pos.x;
		clipInfo.screenY = pos.y;
		clipInfo.sx = pos.x;
		clipInfo.sy = pos.y;
		clipInfo.sw = size.x;
		clipInfo.sh = size.y;

		renderer->SetScissorBox(pos.x, pos.y, size.x, size.y);
		wnd->ApplyBinds();
		wnd->OnDraw(clipInfo);
		renderer->PopMatrix();
	}

	renderer->_SetScissor(false);
}

/*
 *	Predefined Actions
 */

void WindowsManager::QuitApplicationAction(Widget* /*src*/, List<Str>* /*args*/)
{
	GetDevice()->StopRunning();
}

void WindowsManager::CopyProperty(Widget* src, List<Str>* args)
{
	//Input parameters:
	//args[0]: Source of the copy
	//args[1]: Destination of the copy
	//TODO: Parse the source and destination to locate the properties to copy

	List<Str> srcField;
	List<Str> dstField;

	(*args)[0].Split('.', srcField);
	(*args)[1].Split('.', dstField);

	Window* window = src->GetWindow();
	Widget* srcWidget = window->FindChildByName(srcField[0]);
	Widget* dstWidget = window->FindChildByName(dstField[0]);

	if (srcWidget == NULL)
	{
		Log::E("CopyProperty action: Source widget '" + srcField[0] + "' could not be found");
		return;
	}

	if (dstWidget == NULL)
	{
		Log::E("CopyProperty action: Destination widget '" + dstField[0] + "' could not be found");
		return;
	}

	dstWidget->SetPropertyValue(dstField[1], srcWidget->GetPropertyValue(srcField[1]));
}

} //namespace Crown
