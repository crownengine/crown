/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "platform.h"

#if CROWN_PLATFORM_ANDROID

#include "memory.h"
#include "window.h"

namespace crown
{

class WindowAndroid : public Window
{
public:

	WindowAndroid()
	{
	}

	void open(uint16_t /*x*/, uint16_t /*y*/, uint16_t /*width*/, uint16_t /*height*/, uint32_t /*parent*/)
	{
	}

	void close()
	{
	}

	void bgfx_setup()
	{
	}

	void show()
	{
	}

	void hide()
	{
	}

	void resize(uint16_t /*width*/, uint16_t /*height*/)
	{
	}

	void move(uint16_t /*x*/, uint16_t /*y*/)
	{
	}

	void minimize()
	{
	}

	void restore()
	{
	}

	const char* title()
	{
	}

	void set_title (const char* /*title*/)
	{
	}

	void* handle()
	{
	}
};

Window* Window::create(Allocator& a)
{
	return CE_NEW(a, WindowAndroid)();
}

void Window::destroy(Allocator& a, Window& w)
{
	CE_DELETE(a, &w);
}

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
