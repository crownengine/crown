/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "command_line.h"
#include "device_options.h"
#include "log.h"
#include "path.h"
#include <stdlib.h>

namespace crown
{
static void help(const char* msg = NULL)
{
	CE_LOGI(
		"The Flexible Game Engine\n"
		"Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.\n"
		"License: https://github.com/taylor001/crown/blob/master/LICENSE\n"
		"\n"
		"Complete documentation available at https://taylor001.github.io/crown/manual.html\n"
		"\n"
		"Usage:\n"
		"  crown [options]\n"
		"\n"
		"Options:\n"
		"  -h --help                  Display this help.\n"
		"  -v --version               Display engine version.\n"
		"  --source-dir <path>        Use <path> as the source directory for resource compilation.\n"
		"  --bundle-dir <path>        Use <path> as the destination directory for compiled resources.\n"
		"  --boot-dir <path>          Boot the engine with the 'boot.config' from given <path>.\n"
		"  --compile                  Do a full compile of the resources.\n"
		"  --platform <platform>      Compile resources for the given <platform>.\n"
		"      linux\n"
		"      windows\n"
		"      android\n"
		"  --continue                 Run the engine after resource compilation.\n"
		"  --console-port <port>      Set port of the console.\n"
		"  --wait-console             Wait for a console connection before starting up.\n"
		"  --parent-window <handle>   Set the parent window <handle> of the main window.\n"
		"  --server                   Run the engine in server mode.\n"
	);

	if (msg)
		CE_LOGE("Error: %s", msg);
}

DeviceOptions::DeviceOptions(int argc, const char** argv)
	: _argc(argc)
	, _argv(argv)
	, _source_dir(NULL)
	, _bundle_dir(NULL)
	, _boot_dir(NULL)
	, _platform(NULL)
	, _wait_console(false)
	, _do_compile(false)
	, _do_continue(false)
	, _server(false)
	, _parent_window(0)
	, _console_port(CROWN_DEFAULT_CONSOLE_PORT)
	, _window_x(0)
	, _window_y(0)
	, _window_width(CROWN_DEFAULT_WINDOW_WIDTH)
	, _window_height(CROWN_DEFAULT_WINDOW_HEIGHT)
{
}

int DeviceOptions::parse()
{
	CommandLine cl(_argc, _argv);

	if (cl.has_argument("help", 'h'))
	{
		help();
		return EXIT_FAILURE;
	}

	if (cl.has_argument("version", 'v'))
	{
		CE_LOGI(CROWN_VERSION);
		return EXIT_FAILURE;
	}

	_source_dir = cl.get_parameter("source-dir");
	_bundle_dir = cl.get_parameter("bundle-dir");

	_do_compile = cl.has_argument("compile");
	if (_do_compile)
	{
		_platform = cl.get_parameter("platform");
		if (_platform == NULL)
		{
			help("Platform must be specified.");
			return EXIT_FAILURE;
		}
		else if (true
			&& strcmp("android", _platform) != 0
			&& strcmp("linux", _platform) != 0
			&& strcmp("windows", _platform) != 0
			)
		{
			help("Unknown platform.");
			return EXIT_FAILURE;
		}

		if (_source_dir == NULL)
		{
			help("Source dir must be specified.");
			return EXIT_FAILURE;
		}

		if (_bundle_dir == NULL)
		{
			help("Bundle dir must be specified.");
			return EXIT_FAILURE;
		}
	}

	_server = cl.has_argument("server");
	if (_server)
	{
		if (_source_dir == NULL)
		{
			help("Source dir must be specified.");
			return EXIT_FAILURE;
		}
	}

	if (_bundle_dir != NULL)
	{
		if (!path::is_absolute(_bundle_dir))
		{
			help("Bundle dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (_source_dir != NULL)
	{
		if (!path::is_absolute(_source_dir))
		{
			help("Source dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	_do_continue = cl.has_argument("continue");

	_boot_dir = cl.get_parameter("boot-dir");
	if (_boot_dir != NULL)
	{
		if (!path::is_relative(_boot_dir))
		{
			help("Boot dir must be relative.");
			return EXIT_FAILURE;
		}
	}

	_wait_console = cl.has_argument("wait-console");

	const char* parent = cl.get_parameter("parent-window");
	if (parent != NULL)
	{
		if (sscanf(parent, "%u", &_parent_window) != 1)
		{
			help("Parent window is invalid.");
			return EXIT_FAILURE;
		}
	}

	const char* port = cl.get_parameter("console-port");
	if (port != NULL)
	{
		if (sscanf(port, "%hu", &_console_port) != 1)
		{
			help("Console port is invalid.");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

} // namespace crown
