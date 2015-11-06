/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "crown.h"
#include "memory.h"
#include "console_server.h"
#include "bundle_compiler.h"
#include "device.h"
#include "command_line.h"
#include "json_parser.h"
#include "main.h"
#include "audio.h"
#include "physics.h"
#include "disk_filesystem.h"
#include "config.h"
#include "math_utils.h"
#include "profiler.h"
#include "temp_allocator.h"
#include <bgfx.h>

namespace crown
{

static void help(const char* msg = NULL)
{
	if (msg)
	{
		printf("Error: %s\n", msg);
	}

	printf(
		"Usage: crown [options]\n"
		"Options:\n\n"

		"  -h --help                  Show this help.\n"
		"  -v --version               Show version informations.\n"
		"  --bundle-dir <path>        Use <path> as the source directory for compiled resources.\n"
		"  --console-port <port>      Set port of the console.\n"
		"  --parent-window <handle>   Set the parent window <handle> of the main window.\n"
		"                             Used only by tools.\n"

		"\nAvailable only in debug and development builds:\n\n"

		"  --source-dir <path>        Use <path> as the source directory for resource compilation.\n"
		"  --project <name>           Start the project <name>.\n"
		"  --compile                  Do a full compile of the resources.\n"
		"  --platform <platform>      Compile resources for the given <platform>.\n"
		"      Possible values for <platform> are:\n"
		"          linux\n"
		"          windows\n"
		"          android\n"
		"  --continue                 Continue the execution after the resource compilation step.\n"
		"  --wait-console             Wait for a console connection before starting up.\n"
	);
}

bool init(const DeviceOptions& opts)
{
	profiler_globals::init();
	audio_globals::init();
	physics_globals::init();
	bgfx::init();
	device_globals::init(opts);
	return true;
}

void update()
{
	while (!process_events() && device()->is_running())
	{
		profiler_globals::clear();
		console_server_globals::update();
		device()->update();
		bgfx::frame();
		profiler_globals::flush();
	}
}

void shutdown()
{
	device_globals::shutdown();
	bgfx::shutdown();
	physics_globals::shutdown();
	audio_globals::shutdown();
	profiler_globals::shutdown();
}
} // namespace crown
