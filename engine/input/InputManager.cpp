/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include "InputManager.h"
#include "OS.h"
#include "Log.h"
#include "EventBuffer.h"

namespace crown
{

//-----------------------------------------------------------------------------
InputManager::InputManager()
{
}

//-----------------------------------------------------------------------------
InputManager::~InputManager()
{
}

//-----------------------------------------------------------------------------
Keyboard* InputManager::keyboard()
{
	return &m_keyboard;
}

//-----------------------------------------------------------------------------
Mouse* InputManager::mouse()
{
	return &m_mouse;
}

//-----------------------------------------------------------------------------
Touch* InputManager::touch()
{
	return &m_touch;
}

//-----------------------------------------------------------------------------
Accelerometer* InputManager::accelerometer()
{
	return &m_accelerometer;
}

//-----------------------------------------------------------------------------
void InputManager::register_mouse_listener(MouseListener* listener)
{
	m_event_dispatcher.add_mouse_listener(listener);
}

//-----------------------------------------------------------------------------
void InputManager::register_keyboard_listener(KeyboardListener* listener)
{
	m_event_dispatcher.add_keyboard_listener(listener);
}

//-----------------------------------------------------------------------------
void InputManager::register_touch_listener(TouchListener* listener)
{
	m_event_dispatcher.add_touch_listener(listener);
}

//-----------------------------------------------------------------------------
void InputManager::register_accelerometer_listener(AccelerometerListener* listener)
{
	m_event_dispatcher.add_accelerometer_listener(listener);
}

//-----------------------------------------------------------------------------
EventDispatcher* InputManager::get_event_dispatcher()
{
	return &m_event_dispatcher;
}

//-----------------------------------------------------------------------------
void InputManager::frame(uint64_t frame_count)
{
	void* event;
	uint32_t event_type;
	size_t event_size;

	// Update input devices
	m_keyboard.m_current_frame = frame_count;
	m_mouse.m_current_frame = frame_count;

	while (1)
	{
		if ((event = os_event_buffer()->get_next_event(event_type, event_size)) == NULL)
		{
			break;
		}

		switch (event_type)
		{
			case OSET_NONE:
			{
				return;
			}
			case OSET_BUTTON_PRESS:
			case OSET_BUTTON_RELEASE:
			{
				MouseEvent mouse_event;
				mouse_event.x = ((OsMouseEvent*)event)->x;
				mouse_event.y = ((OsMouseEvent*)event)->y;
				mouse_event.button = ((OsMouseEvent*)event)->button == 0 ? MB_LEFT : ((OsMouseEvent*)event)->button == 1 ? MB_MIDDLE : MB_RIGHT;
				mouse_event.wheel = 0.0f;

				if (event_type == OSET_BUTTON_PRESS)
				{
					m_mouse.update(frame_count, mouse_event.button, true);
					m_event_dispatcher.button_pressed(mouse_event);
				}
				else
				{
					m_mouse.update(frame_count, mouse_event.button, false);
					m_event_dispatcher.button_released(mouse_event);
				}

				break;
			}
			case OSET_KEY_PRESS:
			case OSET_KEY_RELEASE:
			{
				KeyboardEvent keyboard_event;
				keyboard_event.key = (KeyCode) ((OsKeyboardEvent*)event)->key;
				keyboard_event.modifier = (uint8_t) ((OsKeyboardEvent*)event)->modifier;

				m_keyboard.m_modifier = keyboard_event.modifier;

				if (event_type == OSET_KEY_PRESS)
				{
					m_keyboard.update(frame_count, keyboard_event.key, true);
					m_event_dispatcher.key_pressed(keyboard_event);
				}
				else
				{
					m_keyboard.update(frame_count, keyboard_event.key, false);
					m_event_dispatcher.key_released(keyboard_event);
				}

				break;
			}
			case OSET_TOUCH_DOWN:
			case OSET_TOUCH_UP:
			{
				TouchEvent touch_event;
				touch_event.pointer_id = ((OsTouchEvent*)event)->pointer_id;
				touch_event.x = ((OsTouchEvent*)event)->x;
				touch_event.y = ((OsTouchEvent*)event)->y;

				m_touch.m_pointers[touch_event.pointer_id].x = touch_event.x;
				m_touch.m_pointers[touch_event.pointer_id].y = touch_event.y;

				// FIXME
				m_touch.m_pointers[touch_event.pointer_id].relative_x = 0.0f;
				m_touch.m_pointers[touch_event.pointer_id].relative_y = 0.0f;

				if (event_type == OSET_TOUCH_DOWN)
				{
					m_touch.m_pointers[touch_event.pointer_id].up = false;
					m_event_dispatcher.touch_down(touch_event);
				}
				else
				{
					m_touch.m_pointers[touch_event.pointer_id].up = true;
					m_event_dispatcher.touch_up(touch_event);
				}

				break;
			}
			case OSET_TOUCH_MOVE:
			{
				TouchEvent touch_event;
				touch_event.pointer_id = ((OsTouchEvent*)event)->pointer_id;
				touch_event.x = ((OsTouchEvent*)event)->x;
				touch_event.y = ((OsTouchEvent*)event)->y;

				m_touch.m_pointers[touch_event.pointer_id].x = touch_event.x;
				m_touch.m_pointers[touch_event.pointer_id].y = touch_event.y;

				// FIXME
				m_touch.m_pointers[touch_event.pointer_id].relative_x = 0.0f;
				m_touch.m_pointers[touch_event.pointer_id].relative_y = 0.0f;

				m_event_dispatcher.touch_move(touch_event);

				break;
			}
			// case OSET_ACCELEROMETER:
			// {
			// 	AccelerometerEvent sensor_event;
			// 	sensor_event.x = event.data_a.float_value;
			// 	sensor_event.y = event.data_b.float_value;
			// 	sensor_event.z = event.data_c.float_value;

			// 	m_accelerometer.m_orientation.x = sensor_event.x;
			// 	m_accelerometer.m_orientation.y = sensor_event.y;
			// 	m_accelerometer.m_orientation.z = sensor_event.z;

			// 	m_event_dispatcher.accelerometer_changed(sensor_event);
			// 	break;
			// }
			default:
			{
				break;
			}
		}
	}
}

} // namespace crown

