/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
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
	const size_t BUFFER_SIZE = 0 +
		+ sizeof(Keyboard)
		+ sizeof(Mouse)
		+ sizeof(Touch);

	char _buffer[BUFFER_SIZE];
	Keyboard* _keyboard = NULL;
	Mouse* _mouse = NULL;
	Touch* _touch = NULL;

	void init()
	{
		_keyboard = new (_buffer) Keyboard();
		_mouse = new (_keyboard + 1) Mouse();
		_touch = new (_mouse + 1) Touch();
	}

	void shutdown()
	{
		_keyboard->~Keyboard();
		_keyboard = NULL;
		_mouse->~Mouse();
		_mouse = NULL;
		_touch->~Touch();
		_touch = NULL;
	}

	void update()
	{
		_keyboard->update();
		_mouse->update();
		_touch->update();
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
