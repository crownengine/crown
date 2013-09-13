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

#include "Crown.h"

namespace crown
{

struct MainArgs
{
	int argc;
	char** argv;
};

static Thread thread("main-thread");

int32_t main_thread(void* data)
{
	crown::os::init_os();

	crown::Device* engine = crown::device();

	MainArgs* args = (MainArgs*)data;
	if (!engine->init(args->argc, args->argv))
	{
		return -1;
	}

	while (engine->is_running())
	{
		engine->frame();
	}

	engine->shutdown();

	thread.stop();

	return 0;
}

int32_t ce_main(void* args)
{
	thread.start(main_thread, args);

	while (thread.is_running());
	
	return 0;
}

}

int32_t main(int argc, char** argv)
{
	crown::MainArgs args;
	args.argc = argc;
	args.argv = argv;
	return crown::ce_main(&args);
}
