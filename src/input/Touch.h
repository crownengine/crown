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

namespace crown
{

class InputManager;

struct TouchEvent
{
	int32_t pointer_id;
	int32_t x;
	int32_t y;
};

/**
	Interface for managing touch input device.	
*/
class TouchListener
{

public:

	virtual void touch_down(const TouchEvent& event) { (void)event; }
	virtual void touch_up(const TouchEvent& event) { (void)event; }
	virtual void touch_move(const TouchEvent& event) { (void)event; }
	virtual void touch_cancel(const TouchEvent& event) { (void)event; }
};

/**
	Interface for accessing touch input device.
*/
class Touch
{

public:

	/**
		Constructor.
	*/
	Touch() : m_listener(NULL) {}

	/**
		Destructor.
	*/
	virtual ~Touch() {}

	/**
		Sets the listener for this device.
	@param listener
		The listener
	*/
	inline void set_listener(TouchListener* listener) { m_listener = listener; }

protected:

	TouchListener* m_listener;
};

}

