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

#include "crown.h"
#include "memory.h"
#include "input.h"
#include "console_server.h"
#include "bundle_compiler.h"
#include "device.h"
#include "command_line.h"
#include "json_parser.h"
#include "keyboard.h"
#include "mouse.h"
#include "touch.h"
#include "main.h"
#include "audio.h"
#include "physics.h"
#include "disk_filesystem.h"
#include "config.h"
#include "math_utils.h"
#include "lua_system.h"
#include <bgfx.h>

namespace crown
{

struct PlatformInfo
{
	const char* name;
	Platform::Enum target;
};

static const PlatformInfo s_platform[Platform::COUNT] =
{
	{ "linux",   Platform::LINUX   },
	{ "windows", Platform::WINDOWS },
	{ "android", Platform::ANDROID }
};

static Platform::Enum string_to_platform(const char* platform)
{
	for (uint32_t i = 0; platform != NULL && i < Platform::COUNT; i++)
	{
		if (string::strcmp(platform, s_platform[i].name) == 0)
			return s_platform[i].target;
	}
	return Platform::COUNT;
}

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
		"  --bundle-dir <path>        Use <path> as the source directory for compiled resources.\n"
		"  --parent-window <handle>   Set the parent window <handle> of the main window.\n"
		"                             Used only by tools.\n"

		"\nAvailable only in debug and development builds:\n\n"

		"  --source-dir <path>        Use <path> as the source directory for resource compilation.\n"
		"  --compile                  Do a full compile of the resources.\n"
		"  --platform <platform>      Compile resources for the given <platform>.\n"
		"      Possible values for <platform> are:\n"
		"          linux\n"
		"          windows\n"
		"          android\n"
		"  --continue                 Continue the execution after the resource compilation step.\n"
		"  --host                     Read resources from a remote engine instance.\n"
		"  --wait-console             Wait for a console connection before starting up.\n"
	);
}

CommandLineSettings parse_command_line(int argc, char** argv)
{
	CommandLineSettings cls;
	cls.source_dir = NULL;
	cls.bundle_dir = NULL;
	cls.platform = Platform::COUNT;
	cls.wait_console = false;
	cls.do_compile = false;
	cls.do_continue = false;
	cls.parent_window = 0;

	CommandLine cmd(argc, argv);

	if (cmd.has_argument("help", 'h'))
	{
		help();
		exit(EXIT_FAILURE);
	}

	cls.source_dir = cmd.get_parameter("source-dir");
	if (!cls.source_dir)
	{
		help("Source directory must be specified.");
		exit(EXIT_FAILURE);
	}
	
	cls.bundle_dir = cmd.get_parameter("bundle-dir");
	if (!cls.bundle_dir)
	{
		help("Bundle directory must be specified.");
		exit(EXIT_FAILURE);
	}

	cls.wait_console = cmd.has_argument("wait-console");
	cls.do_compile = cmd.has_argument("compile");
	cls.do_continue = cmd.has_argument("continue");

	cls.platform = string_to_platform(cmd.get_parameter("platform"));
	if (cls.do_compile && cls.platform == Platform::COUNT)
	{
		help("Platform must be specified.");
		exit(EXIT_FAILURE);
	}

	const char* parent = cmd.get_parameter("parent-window");
	if (parent)
	{
		cls.parent_window = string::parse_uint(parent);
	}

	return cls;
}

ConfigSettings parse_config_file(Filesystem& fs)
{
	ConfigSettings cs;
	cs.console_port = 10001;
	cs.boot_package = 0;
	cs.boot_script = 0;
	cs.window_width = CROWN_DEFAULT_WINDOW_WIDTH;
	cs.window_height = CROWN_DEFAULT_WINDOW_HEIGHT;

	File* tmpfile = fs.open("crown.config", FOM_READ);
	JSONParser config(*tmpfile);
	fs.close(tmpfile);
	JSONElement root = config.root();

	JSONElement cport = root.key_or_nil("console_port");
	if (!cport.is_nil())
	{
		cs.console_port = (int16_t) cport.to_int();
	}

	JSONElement window_width = root.key_or_nil("window_width");
	if (!window_width.is_nil())
	{
		cs.window_width = math::max((uint16_t)1, (uint16_t)window_width.to_int());
	}

	JSONElement window_height = root.key_or_nil("window_height");
	if (!window_height.is_nil())
	{
		cs.window_height = math::max((uint16_t)1, (uint16_t)window_height.to_int());
	}

	cs.boot_script = root.key("boot_script").to_resource_id("lua").name;
	cs.boot_package = root.key("boot_package").to_resource_id("package").name;

	return cs;
}

bool init(Filesystem& fs, const ConfigSettings& cs)
{
	input_globals::init();
	audio_globals::init();
	physics_globals::init();
	bgfx::init();
	lua_globals::init();
	device_globals::init(fs, cs.boot_package, cs.boot_script);
	device()->init();
	return true;
}

void update()
{
	while (!process_events() && device()->is_running())
	{
#if defined(CROWN_DEBUG)
		console_server_globals::console().update();
#endif
		device()->update();
		bgfx::frame();
		lua_globals::clear_temporaries();
		input_globals::keyboard().update();
		input_globals::mouse().update();
		input_globals::touch().update();
	}
}

void shutdown()
{
	device()->shutdown();
	device_globals::shutdown();
	lua_globals::shutdown();
	bgfx::shutdown();
	physics_globals::shutdown();
	audio_globals::shutdown();
	input_globals::shutdown();
}
} // namespace crown
