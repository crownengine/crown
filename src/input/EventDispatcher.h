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

#include "List.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Touch.h"
#include "Accelerometer.h"

namespace crown
{

class EventDispatcher
{
	typedef List<MouseListener*>			MouseListenerList;
	typedef List<KeyboardListener*>			KeyboardListenerList;
	typedef List<TouchListener*>			TouchListenerList;
	typedef List<AccelerometerListener*>	AccelerometerListenerList;

public:

	EventDispatcher();
	~EventDispatcher();

	void add_mouse_listener(MouseListener* listener);
	void add_keyboard_listener(KeyboardListener* listener);
	void add_touch_listener(TouchListener* listener);
	void add_accelerometer_listener(AccelerometerListener* listener);

	void button_pressed(const MouseEvent&);
	void button_released(const MouseEvent&);
	void cursor_moved(const MouseEvent&);

	void key_pressed(const KeyboardEvent&);
	void key_released(const KeyboardEvent&);
	void text_input(const KeyboardEvent&);

	void touch_down(const TouchEvent& event);
	void touch_up(const TouchEvent& event);
	void touch_move(const TouchEvent& event);
	void touch_cancel(const TouchEvent& event);

	void accelerometer_changed(const AccelerometerEvent& event);

private:

	MouseListenerList			m_mouse_listener_list;
	KeyboardListenerList		m_keyboard_listener_list;
	TouchListenerList			m_touch_listener_list;
	AccelerometerListenerList	m_acc_listener_list;
};

} // namespace crown

