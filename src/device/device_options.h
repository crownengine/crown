/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/option.h"
#include "core/strings/dynamic_string.h"
#include "core/types.h"
#include "device/boot_config.h"

namespace crown
{
struct DisplayServer
{
	enum Enum
	{
		WAYLAND,
		X11,

		COUNT
	};
};

/// Holds device options.
///
/// @ingroup Device
struct DeviceOptions
{
	int _argc;
	const char **_argv;
	DynamicString _source_dir;
	const char *_map_source_dir_name;
	DynamicString _map_source_dir_prefix;
	DynamicString _data_dir;
	DynamicString _bundle_dir;
	DynamicString _port_file;
	const char *_boot_dir;
	const char *_platform;
	DynamicString _lua_string;
	bool _wait_console;
	bool _do_compile;
	bool _do_continue;
	bool _do_bundle;
	bool _server;
	bool _pumped;
	bool _hidden;
	bool _keep_above;
	u32 _parent_window;
	u16 _console_port;
	u16 _window_x;
	u16 _window_y;
	Option<u16> _window_width;
	Option<u16> _window_height;
	Option<RendererType::Enum> _renderer_type;
	Option<DisplayServer::Enum> _display_server;

	///
	DeviceOptions(Allocator &a, int argc, const char **argv);

	///
	~DeviceOptions();

	/// Parses the command line and returns
	/// EXIT_SUCCESS if no error is found.
	int parse(bool *quit);
};

} // namespace crown
