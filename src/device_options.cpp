/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "device_options.h"
#include "command_line.h"

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

DeviceOptions::DeviceOptions(int argc, char** argv)
	: _source_dir(NULL)
	, _bundle_dir(NULL)
	, _project(NULL)
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
	CommandLine cmd(argc, argv);

	_source_dir = cmd.get_parameter("source-dir");
	_bundle_dir = cmd.get_parameter("bundle-dir");
	_project = cmd.get_parameter("project");
	_platform = cmd.get_parameter("platform");

	_wait_console = cmd.has_argument("wait-console");
	_do_compile = cmd.has_argument("compile");
	_do_continue = cmd.has_argument("continue");

	const char* parent = cmd.get_parameter("parent-window");
	if (parent)
		_parent_window = parse_uint(parent);

	const char* port = cmd.get_parameter("console-port");
	if (port)
		_console_port = parse_uint(port);
}

}

