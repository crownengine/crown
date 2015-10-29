/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "device_options.h"
#include "command_line.h"

namespace crown
{

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

