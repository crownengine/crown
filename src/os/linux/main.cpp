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

#include "Crown.h"
#include "List.h"
#include "GLXContext.h"
#include <X11/Xlib.h>

namespace crown
{
namespace os
{
extern Window window;
}
}

int main(int argc, char** argv)
{
	crown::os::init_os();

	crown::os::create_render_window(0, 0, 1000, 625, false);

	crown::Context context;
	context.set_window(crown::os::window);
	context.create_context();

	crown::Device* engine = crown::device();

	if (!engine->init(argc, argv))
	{
		exit(-1);
	}

	// Main loop
	while (engine->is_running())
	{
		engine->frame();

		crown::os::swap_buffers();
	}

	engine->shutdown();

	crown::os::destroy_render_window();

	return 0;
}

