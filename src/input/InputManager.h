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

#include "EventDispatcher.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Touch.h"
#include "Accelerometer.h"

namespace crown
{

class MouseListener;
class KeyboardListener;
class TouchListener;

class InputManager
{
public:

						InputManager();
						~InputManager();

	Keyboard*			keyboard();
	Mouse*				mouse();
	Touch*				touch();
	Accelerometer*		accelerometer();

	void				register_mouse_listener(MouseListener* listener);
	void				register_keyboard_listener(KeyboardListener* listener);
	void				register_touch_listener(TouchListener* listener);
	void				register_accelerometer_listener(AccelerometerListener* listener);

	EventDispatcher*	get_event_dispatcher();

	/// Returns whether the cursor is visible.
	bool				is_cursor_visible() const;

	/// Sets whether the cursor is visible.
	void				set_cursor_visible(bool visible);

	void				event_loop();

private:

	EventDispatcher		m_event_dispatcher;

	Keyboard			m_keyboard;
	Mouse				m_mouse;
	Touch				m_touch;
	Accelerometer		m_accelerometer;

	bool				m_cursor_visible;
};

} // namespace crown

