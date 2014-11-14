/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "filesystem_types.h"

namespace crown
{
	struct Platform
	{
		enum Enum
		{
			LINUX = 0,
			WINDOWS = 1,
			ANDROID = 2,

			COUNT
		};
	};

	struct ConfigSettings
	{
		ConfigSettings()
			: source_dir(NULL)
			, bundle_dir(NULL)
			, project(NULL)
			, platform(Platform::COUNT)
			, wait_console(false)
			, do_compile(false)
			, do_continue(false)
			, parent_window(0)
			, console_port(10001)
			, boot_package(0)
			, boot_script(0)
			, window_width(CROWN_DEFAULT_WINDOW_WIDTH)
			, window_height(CROWN_DEFAULT_WINDOW_HEIGHT)
		{
		}

		const char* source_dir;
		const char* bundle_dir;
		const char* project;
		Platform::Enum platform;
		bool wait_console;
		bool do_compile;
		bool do_continue;
		uint32_t parent_window;
		uint16_t console_port;
		StringId64 boot_package;
		StringId64 boot_script;
		uint16_t window_width;
		uint16_t window_height;
	};

	void parse_command_line(int argc, char** argv, ConfigSettings& cs);

	/// Read configuration file from @a fs.
	void parse_config_file(Filesystem& fs, ConfigSettings& cs);

	/// Initializes the engine.
	bool init(Filesystem& fs, const ConfigSettings& cs);

	/// Updates all the subsystems.
	void update();

	/// Shutdowns the engine.
	void shutdown();
} // namespace crown
