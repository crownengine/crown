/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/option.h"
#include "core/platform.h"
#include "core/strings/dynamic_string.h"
#include "core/types.h"

namespace crown
{
/// Holds device options.
///
/// @ingroup Device
struct DeviceOptions
{
	Option<int> _argc;
	Option<const char **> _argv;
	Option<DynamicString> _source_dir;
	Option<const char *> _map_source_dir_name;
	Option<DynamicString> _map_source_dir_prefix;
	Option<DynamicString> _data_dir;
	Option<DynamicString> _bundle_dir;
	Option<const char *> _boot_dir;
	Option<const char *> _platform;
	Option<DynamicString> _lua_string;
	Option<bool> _wait_console;
	Option<bool> _do_compile;
	Option<bool> _do_continue;
	Option<bool> _do_bundle;
	Option<bool> _server;
	Option<bool> _pumped;
	Option<bool> _hidden;
	Option<u32> _parent_window;
	Option<u16> _console_port;
	Option<u16> _window_x;
	Option<u16> _window_y;
	Option<u16> _window_width;
	Option<u16> _window_height;
#if CROWN_PLATFORM_ANDROID
	Option<void *> _asset_manager;
#endif

	///
	DeviceOptions(Allocator &a, int argc, const char **argv);

	///
	~DeviceOptions();

	/// Parses the command line and returns
	/// EXIT_SUCCESS if no error is found.
	int parse(bool *quit);
};

} // namespace crown
