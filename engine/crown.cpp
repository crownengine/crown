/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
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
#include "profiler.h"
#include "temp_allocator.h"
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
	{ "android", Platform::ANDROID },
	{ "osx",     Platform::OSX     }
};

static Platform::Enum string_to_platform(const char* platform)
{
	for (uint32_t i = 0; platform != NULL && i < Platform::COUNT; i++)
	{
		if (strcmp(platform, s_platform[i].name) == 0)
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
		"          osx\n"
		"  --continue                 Continue the execution after the resource compilation step.\n"
		"  --host                     Read resources from a remote engine instance.\n"
		"  --wait-console             Wait for a console connection before starting up.\n"
	);
}

void parse_command_line(int argc, char** argv, ConfigSettings& cs)
{
	CommandLine cmd(argc, argv);

	if (cmd.has_argument("help", 'h'))
	{
		help();
		exit(EXIT_SUCCESS);
	}

	if (cmd.has_argument("version", 'v'))
	{
		printf(CROWN_PLATFORM_NAME "-" CROWN_CPU_NAME " " "("CROWN_ARCH_NAME")" " " "("CROWN_COMPILER_NAME")\n");
		exit(EXIT_SUCCESS);
	}

	cs.source_dir = cmd.get_parameter("source-dir");
	if (!cs.source_dir)
	{
		help("Source directory must be specified.");
		exit(EXIT_FAILURE);
	}

	cs.bundle_dir = cmd.get_parameter("bundle-dir");
	if (!cs.bundle_dir)
	{
		help("Bundle directory must be specified.");
		exit(EXIT_FAILURE);
	}

	if (strcmp(cs.source_dir, cs.bundle_dir) == 0)
	{
		help("Source and Bundle directories must differ.");
		exit(EXIT_FAILURE);
	}

	cs.project = cmd.get_parameter("project");

	cs.wait_console = cmd.has_argument("wait-console");
	cs.do_compile = cmd.has_argument("compile");
	cs.do_continue = cmd.has_argument("continue");

	cs.platform = string_to_platform(cmd.get_parameter("platform"));
	if (cs.do_compile && cs.platform == Platform::COUNT)
	{
		help("Platform must be specified.");
		exit(EXIT_FAILURE);
	}

	const char* parent = cmd.get_parameter("parent-window");
	if (parent)
	{
		cs.parent_window = parse_uint(parent);
	}
}

void parse_config_file(Filesystem& fs, ConfigSettings& cs)
{
	TempAllocator512 alloc;
	DynamicString project_path(alloc);

	if (cs.project != NULL)
	{
		project_path += cs.project;
		project_path += "/";
	}
	project_path += "crown.config";

	File* tmpfile = fs.open(project_path.c_str(), FOM_READ);
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
		cs.window_width = max((uint16_t)1, (uint16_t)window_width.to_int());
	}

	JSONElement window_height = root.key_or_nil("window_height");
	if (!window_height.is_nil())
	{
		cs.window_height = max((uint16_t)1, (uint16_t)window_height.to_int());
	}

	cs.boot_script = root.key("boot_script").to_resource_id("lua").name;
	cs.boot_package = root.key("boot_package").to_resource_id("package").name;
}

bool init(Filesystem& fs, const ConfigSettings& cs)
{
	profiler_globals::init();
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
		profiler_globals::clear();
		console_server_globals::update();
		device()->update();
		bgfx::frame();
		lua_globals::clear_temporaries();
		input_globals::update();
		profiler_globals::flush();
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
	profiler_globals::shutdown();
}
} // namespace crown
