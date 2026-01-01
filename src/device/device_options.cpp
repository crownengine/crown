/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/command_line.h"
#include "core/filesystem/path.h"
#include "core/option.inl"
#include "core/strings/dynamic_string.inl"
#include "device/device_options.h"
#include <errno.h>
#include <stdio.h>  // printf
#include <stdlib.h> // EXIT_SUCCESS, strto* etc.

namespace crown
{
static void help(const char *msg = NULL)
{
	printf(
		"The Flexible Game Engine\n"
		"Copyright (c) 2012-2026 Daniele Bartolini et al.\n"
		"SPDX-License-Identifier: MIT\n"
		"\n"
		"Usage:\n"
		"  crown [options]\n"
		"\n"
		"Options:\n"
		"  --                              End of the runtime's options.\n"
		"  -h --help                       Display this help.\n"
		"  -v --version                    Display engine version.\n"
		"  --source-dir <path>             Specify the <path> of the project's source data.\n"
		"  --data-dir <path>               Run with the data located at <path>.\n"
		"  --bundle-dir <path>             Run with the bundles located at <path>.\n"
		"  --map-source-dir <name> <path>  Mount <path>/<name> at <source-dir>/<name>.\n"
		"  --boot-dir <prefix>             Use <prefix>/boot.config to boot the engine.\n"
		"  --compile                       Compile the project's source data.\n"
		"  --bundle                        Generate bundles after the data has been compiled.\n"
		"  --platform <platform>           Specify the target <platform> for data compilation.\n"
		"      android\n"
		"      html5\n"
		"      linux\n"
		"      windows\n"
		"  --continue                      Run the engine after the data has been compiled.\n"
		"  --console-port <port>           Set port of the console server.\n"
		"  --wait-console                  Wait for a console connection before booting the engine.\n"
		"  --parent-window <handle>        Set the parent window <handle> of the main window.\n"
		"  --server                        Run the engine in server mode.\n"
		"  --pumped                        Do not advance the renderer unless explicitly requested via console.\n"
		"  --hidden                        Make the main window initially invisible.\n"
		"  --window-rect <x y w h>         Sets the main window's position and size.\n"
		"  --string-id <string>            Prints the 32- and 64-bits IDs of <string>.\n"
		"  --run-unit-tests                Run unit tests and quit.\n"
		"\n"
		"Full documentation at https://docs.crownengine.org/html/v" CROWN_MANUAL_VERSION "/reference/command_line.html\n"
		);

	if (msg)
		printf("Error: %s\n", msg);
}

DeviceOptions::DeviceOptions(Allocator &a, int argc, const char **argv)
	: _argc(argc)
	, _argv(argv)
	, _source_dir(DynamicString(a))
	, _map_source_dir_name(NULL)
	, _map_source_dir_prefix(DynamicString(a))
	, _data_dir(DynamicString(a))
	, _bundle_dir(DynamicString(a))
	, _boot_dir(NULL)
	, _platform(NULL)
	, _lua_string(DynamicString(a))
	, _wait_console(false)
	, _do_compile(false)
	, _do_continue(false)
	, _do_bundle(false)
	, _server(false)
	, _pumped(false)
	, _hidden(false)
	, _parent_window(0)
	, _console_port(CROWN_DEFAULT_CONSOLE_PORT)
	, _window_x(0)
	, _window_y(0)
	, _window_width(CROWN_DEFAULT_WINDOW_WIDTH)
	, _window_height(CROWN_DEFAULT_WINDOW_HEIGHT)
#if CROWN_PLATFORM_ANDROID
	, _asset_manager(NULL)
#endif
{
}

DeviceOptions::~DeviceOptions()
{
}

int DeviceOptions::parse(bool *quit)
{
	CommandLine cl(_argc, _argv);

	if (cl.has_option("help", 'h')) {
		help();
		*quit = true;
		return EXIT_SUCCESS;
	}

	if (cl.has_option("version", 'v')) {
		printf("Crown " CROWN_VERSION "\n");
		*quit = true;
		return EXIT_SUCCESS;
	}

	path::reduce(_source_dir, cl.get_parameter(0, "source-dir"));
	path::reduce(_data_dir, cl.get_parameter(0, "data-dir"));
	path::reduce(_bundle_dir, cl.get_parameter(0, "bundle-dir"));

	_map_source_dir_name = cl.get_parameter(0, "map-source-dir");
	if (_map_source_dir_name) {
		path::reduce(_map_source_dir_prefix, cl.get_parameter(1, "map-source-dir"));
		if (_map_source_dir_prefix.empty()) {
			help("Mapped source directory must be specified.");
			return EXIT_FAILURE;
		}
	}

	_do_compile = cl.has_option("compile");
	_do_bundle = cl.has_option("bundle");
	if (_do_compile || _do_bundle) {
		_platform = cl.get_parameter(0, "platform");

		// Compile for platform the executable is built for.
		if (_platform == NULL)
			_platform = CROWN_PLATFORM_NAME;

		if (_source_dir.empty()) {
			help("Source dir must be specified.");
			return EXIT_FAILURE;
		}

		if (_data_dir.empty()) {
			_data_dir += _source_dir;
			_data_dir += '_';
			_data_dir += _platform;
		}

		if (_do_bundle) {
			if (_bundle_dir.empty()) {
				_bundle_dir += _source_dir;
				_bundle_dir += "_bundle_";
				_bundle_dir += _platform;
			}
		}
	}

	_server = cl.has_option("server");
	if (_server) {
		if (_source_dir.empty()) {
			help("Source dir must be specified.");
			return EXIT_FAILURE;
		}
	}

	_pumped = cl.has_option("pumped");
	_hidden = cl.has_option("hidden");

	if (!_data_dir.empty()) {
		if (!path::is_absolute(_data_dir.c_str())) {
			help("Data dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (!_source_dir.empty()) {
		if (!path::is_absolute(_source_dir.c_str())) {
			help("Source dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	if (!_map_source_dir_prefix.empty()) {
		if (!path::is_absolute(_map_source_dir_prefix.c_str())) {
			help("Mapped source dir must be absolute.");
			return EXIT_FAILURE;
		}
	}

	_do_continue = cl.has_option("continue");
	if (_do_continue) {
		if (strcmp(_platform, CROWN_PLATFORM_NAME) != 0) {
			help("Host platform can not run from the compiled data. Consider removing --continue option.");
			return EXIT_FAILURE;
		}
	}

	_boot_dir = cl.get_parameter(0, "boot-dir");
	if (_boot_dir) {
		if (!path::is_relative(_boot_dir)) {
			help("Boot dir must be relative.");
			return EXIT_FAILURE;
		}
	}

	_wait_console = cl.has_option("wait-console");

	const char *parent = cl.get_parameter(0, "parent-window");
	if (parent) {
		errno = 0;
		_parent_window = strtoul(parent, NULL, 10);
		if (errno == ERANGE || errno == EINVAL) {
			help("Parent window is invalid.");
			return EXIT_FAILURE;
		}
	}

	const char *port = cl.get_parameter(0, "console-port");
	if (port) {
		errno = 0;
		_console_port = (u16)strtoul(port, NULL, 10);
		if (errno == ERANGE || errno == EINVAL) {
			help("Console port is invalid.");
			return EXIT_FAILURE;
		}
	}

	const char *ls = cl.get_parameter(0, "lua-string");
	if (ls)
		_lua_string = ls;

	if (cl.has_option("string-id")) {
		const char *string_id_utf8 = cl.get_parameter(0, "string-id");
		if (string_id_utf8 == NULL) {
			help("Usage: string-id <utf8>");
			*quit = true;
			return EXIT_FAILURE;
		} else {
			char buf[STRING_ID64_BUF_LEN];
			printf("STRING_ID_32(\"%s\", UINT32_C(0x%s))\n"
				, string_id_utf8
				, StringId32(string_id_utf8).to_string(buf, sizeof(buf))
				);
			printf("STRING_ID_64(\"%s\", UINT64_C(0x%s))\n"
				, string_id_utf8
				, StringId64(string_id_utf8).to_string(buf, sizeof(buf))
				);
			*quit = true;
			return EXIT_SUCCESS;
		}
	}

	if (cl.has_option("window-rect")) {
		const char *rect[4];
		for (u32 ii = 0; ii < countof(rect); ++ii) {
			rect[ii] = cl.get_parameter(ii, "window-rect");

			if (!rect[ii]) {
				printf("window-rect: format must be: <x y w h>\n");
				*quit = true;
				return EXIT_FAILURE;
			}
		}

		_window_x = (u16)strtoul(rect[0], NULL, 10);
		_window_y = (u16)strtoul(rect[1], NULL, 10);
		_window_width.set_value((u16)strtoul(rect[2], NULL, 10));
		_window_height.set_value((u16)strtoul(rect[3], NULL, 10));
	}

	return EXIT_SUCCESS;
}

} // namespace crown
