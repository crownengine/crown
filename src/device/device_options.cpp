/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/command_line.h"
#include "core/filesystem/path.h"
#include "core/strings/dynamic_string.inl"
#include "device/device_options.h"
#include <stdlib.h>

namespace crown
{
static void help(const char* msg = NULL)
{
	printf(
		"The Flexible Game Engine\n"
		"Copyright (c) 2012-2021 Daniele Bartolini et al.\n"
		"License: https://github.com/dbartolini/crown/blob/master/LICENSE\n"
		"\n"
		"Usage:\n"
		"  crown [options]\n"
		"\n"
		"Options:\n"
		"  -h --help                       Display this help.\n"
		"  -v --version                    Display engine version.\n"
		"  --source-dir <path>             Specify the <path> of the project's source data.\n"
		"  --data-dir <path>               Specify the <path> where to put the compiled data.\n"
		"  --map-source-dir <name> <path>  Mount <path>/<name> at <source-dir>/<name>.\n"
		"  --boot-dir <prefix>             Use <prefix>/boot.config to boot the engine.\n"
		"  --compile                       Compile the project's source data.\n"
		"  --platform <platform>           Specify the target <platform> for data compilation.\n"
		"      linux\n"
		"      windows\n"
		"      android\n"
		"  --continue                      Run the engine after the data has been compiled.\n"
		"  --console-port <port>           Set port of the console server.\n"
		"  --wait-console                  Wait for a console connection before booting the engine.\n"
		"  --parent-window <handle>        Set the parent window <handle> of the main window.\n"
		"  --server                        Run the engine in server mode.\n"
		"  --pumped                        Do not advance the renderer unless explicitly requested via console.\n"
		"\n"
		"Complete documentation available at https://dbartolini.github.io/crown/html/v" CROWN_VERSION "\n"
	);

	if (msg)
		printf("Error: %s\n", msg);
}

DeviceOptions::DeviceOptions(Allocator& a, int argc, const char** argv)
	: _argc(argc)
	, _argv(argv)
	, _source_dir(a)
	, _map_source_dir_name(NULL)
	, _map_source_dir_prefix(a)
	, _data_dir(a)
	, _boot_dir(NULL)
	, _platform(NULL)
	, _lua_string(a)
	, _wait_console(false)
	, _do_compile(false)
	, _do_continue(false)
	, _server(false)
	, _pumped(false)
	, _parent_window(0)
	, _console_port(CROWN_DEFAULT_CONSOLE_PORT)
	, _window_x(0)
	, _window_y(0)
	, _window_width(CROWN_DEFAULT_WINDOW_WIDTH)
	, _window_height(CROWN_DEFAULT_WINDOW_HEIGHT)
{
}

DeviceOptions::~DeviceOptions()
{
}

int DeviceOptions::parse(bool* quit)
{
	CommandLine cl(_argc, _argv);

	if (cl.has_option("help", 'h'))
	{
		help();
		*quit = true;
		return EXIT_SUCCESS;
	}

	if (cl.has_option("version", 'v'))
	{
		printf("Crown " CROWN_VERSION "\n");
		*quit = true;
		return EXIT_SUCCESS;
	}

	path::reduce(_source_dir, cl.get_parameter(0, "source-dir"));
	path::reduce(_data_dir, cl.get_parameter(0, "data-dir"));

	_map_source_dir_name = cl.get_parameter(0, "map-source-dir");
	if (_map_source_dir_name)
	{
		path::reduce(_map_source_dir_prefix, cl.get_parameter(1, "map-source-dir"));
		if (_map_source_dir_prefix.empty())
		{
			help("Mapped source directory must be specified.");
			return EXIT_FAILURE;
		}
	}

	_do_compile = cl.has_option("compile");
	if (_do_compile)
	{
		_platform = cl.get_parameter(0, "platform");

		// Compile for platform the executable is built for.
		if (_platform == NULL)
			_platform = CROWN_PLATFORM_NAME;

		if (true
			&& strcmp(_platform, "android") != 0
			&& strcmp(_platform, "linux") != 0
			&& strcmp(_platform, "windows") != 0
			)
		{
			help("Cannot compile for the given platform.");
			return EXIT_FAILURE;
		}

		if (_source_dir.empty())
		{
			help("Source dir must be specified.");
			return EXIT_FAILURE;
		}

		if (_data_dir.empty())
		{
			_data_dir += _source_dir;
			_data_dir += '_';
			_data_dir += _platform;
		}
	}

	_server = cl.has_option("server");
	if (_server)
	{
		if (_source_dir.empty())
		{
			help("Source dir must be specified.");
			return EXIT_FAILURE;
		}
	}

	_pumped = cl.has_option("pumped");

	if (!_data_dir.empty())
	{
		if (!path::is_absolute(_data_dir.c_str()))
		{
			help("Data dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (!_source_dir.empty())
	{
		if (!path::is_absolute(_source_dir.c_str()))
		{
			help("Source dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (!_map_source_dir_prefix.empty())
	{
		if (!path::is_absolute(_map_source_dir_prefix.c_str()))
		{
			help("Mapped source dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	_do_continue = cl.has_option("continue");
	if (_do_continue)
	{
		if (strcmp(_platform, CROWN_PLATFORM_NAME) != 0)
		{
			help("Host platform can not run from the compiled data. Consider removing --continue option.");
			return EXIT_FAILURE;
		}
	}

	_boot_dir = cl.get_parameter(0, "boot-dir");
	if (_boot_dir)
	{
		if (!path::is_relative(_boot_dir))
		{
			help("Boot dir must be relative.");
			return EXIT_FAILURE;
		}
	}

	_wait_console = cl.has_option("wait-console");

	const char* parent = cl.get_parameter(0, "parent-window");
	if (parent)
	{
		if (sscanf(parent, "%u", &_parent_window) != 1)
		{
			help("Parent window is invalid.");
			return EXIT_FAILURE;
		}
	}

	const char* port = cl.get_parameter(0, "console-port");
	if (port)
	{
		if (sscanf(port, "%hu", &_console_port) != 1)
		{
			help("Console port is invalid.");
			return EXIT_FAILURE;
		}
	}

	const char* ls = cl.get_parameter(0, "lua-string");
	if (ls)
		_lua_string = ls;

	return EXIT_SUCCESS;
}

} // namespace crown
