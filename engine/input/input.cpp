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

#include "input.h"
#include "keyboard.h"
#include "mouse.h"
#include "touch.h"
#include "memory.h"

namespace crown
{
namespace input_globals
{
	Keyboard* _keyboard = NULL;
	Mouse* _mouse = NULL;
	Touch* _touch = NULL;

	void init()
	{
		_keyboard = CE_NEW(default_allocator(), Keyboard);
		_mouse = CE_NEW(default_allocator(), Mouse);
		_touch = CE_NEW(default_allocator(), Touch);
	}

	void shutdown()
	{
		CE_DELETE(default_allocator(), _keyboard);
		CE_DELETE(default_allocator(), _mouse);
		CE_DELETE(default_allocator(), _touch);
	}

	Keyboard& keyboard()
	{
		return *_keyboard;
	}

	Mouse& mouse()
	{
		return *_mouse;
	}

	Touch& touch()
	{
		return *_touch;
	}
} // namespace input_globals
} // namespace crown
