/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "device_options.h"
#include "command_line.h"
#include "log.h"
#include "path.h"

namespace crown
{

static void help(const char* msg = NULL)
{
	if (msg)
	{
		CE_LOGE("Error: %s\n", msg);
	}

	CE_LOGI(
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
		"  --boot-dir <path>          Boot the engine with the 'boot.config' from given <path>.\n"
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

DeviceOptions::DeviceOptions(int argc, char** argv)
	: _argc(argc)
	, _argv(argv)
	, _source_dir(NULL)
	, _bundle_dir(NULL)
	, _boot_dir(NULL)
	, _platform(NULL)
	, _wait_console(false)
	, _do_compile(false)
	, _do_continue(false)
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
		CE_LOGI(CROWN_VERSION_STRING);
		return EXIT_FAILURE;
	}

	_source_dir = cl.get_parameter("source-dir");
	if (_source_dir == NULL)
	{
		help("Source dir must be specified.");
		return EXIT_FAILURE;
	}

	_bundle_dir = cl.get_parameter("bundle-dir");
	if (_bundle_dir == NULL)
	{
		help("Bundle dir must be specified.");
		return EXIT_FAILURE;
	}

	if (!path::is_absolute(_source_dir))
	{
		help("Source dir must be absolute");
		return EXIT_FAILURE;
	}

	if (!path::is_absolute(_bundle_dir))
	{
		help("Bundle dir must be absolute");
		return EXIT_FAILURE;
	}

	_do_compile = cl.has_argument("compile");
	_platform = cl.get_parameter("platform");
	if (_do_compile && _platform == NULL)
	{
		help("Platform must be specified.");
		return EXIT_FAILURE;
	}

	_do_continue = cl.has_argument("continue");

	_boot_dir = cl.get_parameter("boot-dir");
	_wait_console = cl.has_argument("wait-console");

	const char* parent = cl.get_parameter("parent-window");
	if (parent != NULL)
	{
		_parent_window = parse_uint(parent);
	}

	const char* port = cl.get_parameter("console-port");
	if (port != NULL)
	{
		_console_port = parse_uint(port);
	}

	return EXIT_SUCCESS;
}

} // namespace crown
