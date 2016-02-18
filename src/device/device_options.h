/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "platform.h"

namespace crown
{
/// Holds device options.
///
/// @ingroup Device
struct DeviceOptions
{
	int _argc;
	char** _argv;
	const char* _source_dir;
	const char* _bundle_dir;
	const char* _boot_dir;
	const char* _platform;
	bool _wait_console;
	bool _do_compile;
	bool _do_continue;
	u32 _parent_window;
	u16 _console_port;
	u16 _window_x;
	u16 _window_y;
	u16 _window_width;
	u16 _window_height;

#if CROWN_PLATFORM_ANDROID
	void* _asset_manager;
#endif // CROWN_PLATFORM_ANDROID

	DeviceOptions(int argc, char** argv);

	/// Parses the command line and returns
	/// EXIT_SUCCESS if no error is found.
	int parse();
};

} // namespace crown
