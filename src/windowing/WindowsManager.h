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
#include "Window.h"
#include "SceneNode.h"
#include "Delegate.h"
#include "KeyCode.h"
#include "Keyboard.h"
#include "Mouse.h"

namespace Crown
{

class Scene;

class WindowsManager: public SceneNode, public WeakReferenced, public MouseListener, public KeyboardListener
{
public:
	WindowsManager(Scene* creator);
	virtual ~WindowsManager();

	virtual void KeyPressed(const KeyboardEvent& event);
	virtual void KeyReleased(const KeyboardEvent& event);
	virtual void TextInput(const KeyboardEvent& event);

	virtual void ButtonPressed(const MouseEvent& event);
	virtual void ButtonReleased(const MouseEvent& event);
	virtual void CursorMoved(const MouseEvent& event);

	virtual void OnRegisterForRendering();

	virtual void Render();

	inline Widget* GetTextInputFocusWidget() const
	{
		return mInputFocusWidget;
	}

	void RegisterAction(const Str& name, ActionDelegate* slot);
	ActionDelegate* GetRegisteredAction(const Str& name);

	void _SetTextInputFocusWidget(Widget* widget);

	bool DoModalWindow(Window* window);

protected:

	bool MouseCapture(Widget* w);
	void MouseRelease();

	void ReleaseWidget(Widget* window);
	void BringWindowToFront(Window* window);

	friend class Widget;

private:
	List<Window*> mWindows;
	Widget* mMouseCapturedWidget;
	Widget* mWidgetUnderMouse;
	Widget* mWidgetMouseDown;     //The widget on which the mouse was pressed (if any)
	Widget* mInputFocusWidget;
	Window* mModalWindow;
	Point2 mMousePosition;
	ActionDictionary mRegisteredActions;

	//Default action handlers
	void QuitApplicationAction(Widget* src, List<Str>* args);
	void CopyProperty(Widget* src, List<Str>* args);

	Widget* FindWidgetAt(Widget* parent, Point2& point) const;
	void LaunchEvent(Widget* targetWidget, IDelegate1<void, bool>* onEventHelper, WindowingEventArgs* args);

	//Helpers
	void MouseInHelper(Widget* targetWidget);
	void MouseOutHelper(Widget* targetWidget);
	void MouseMoveHelper(Widget* targetWidget, const Point2& delta);
	void MouseUpHelper(Widget* targetWidget, bool);
	void MouseDownHelper(Widget* targetWidget);
	void TextInputHelper(Widget* targetWidget, const Str& text);
	void GotFocusHelper(Widget* targetWidget);
	void LostFocusHelper(Widget* targetWidget);
	void KeyDownHelper(Widget* targetWidget, Key key);
	void KeyUpHelper(Widget* targetWidget, Key key);
	void SizeChangedHelper(Widget* targetWidget);

	//Called from Window::Close() to end modal
	void EndModal(Window* window);

	friend class Window;
};

} //namespace Crown
