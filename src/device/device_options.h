/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

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
	int _argc;
	const char** _argv;
	DynamicString _source_dir;
	const char* _map_source_dir_name;
	DynamicString _map_source_dir_prefix;
	DynamicString _data_dir;
	const char* _boot_dir;
	const char* _platform;
	DynamicString _lua_string;
	bool _wait_console;
	bool _do_compile;
	bool _do_continue;
	bool _server;
	bool _pumped;
	u32 _parent_window;
	u16 _console_port;
	u16 _window_x;
	u16 _window_y;
	u16 _window_width;
	u16 _window_height;

#if CROWN_PLATFORM_ANDROID
	void* _asset_manager;
#endif // CROWN_PLATFORM_ANDROID

	///
	DeviceOptions(Allocator& a, int argc, const char** argv);

	///
	~DeviceOptions();

	/// Parses the command line and returns
	/// EXIT_SUCCESS if no error is found.
	int parse(bool* quit);
};

} // namespace crown
