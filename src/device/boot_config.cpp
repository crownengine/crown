/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/platform.h"
#include "core/strings/dynamic_string.inl"
#include "device/boot_config.h"

namespace crown
{
BootConfig::BootConfig(Allocator& a)
	: boot_script_name(a)
	, boot_package_name(u64(0))
	, window_title(a)
	, window_w(CROWN_DEFAULT_WINDOW_WIDTH)
	, window_h(CROWN_DEFAULT_WINDOW_HEIGHT)
	, aspect_ratio(-1.0f)
	, vsync(true)
	, fullscreen(false)
{
}

bool BootConfig::parse(const char* json)
{
	TempAllocator4096 ta;
	JsonObject cfg(ta);
	sjson::parse(cfg, json);

	// General configs
	sjson::parse_string(boot_script_name, cfg["boot_script"]);
	boot_package_name = sjson::parse_resource_name(cfg["boot_package"]);

	if (json_object::has(cfg, "window_title"))
		sjson::parse_string(window_title, cfg["window_title"]);

	// Platform-specific configs
	if (json_object::has(cfg, CROWN_PLATFORM_NAME))
	{
		JsonObject platform(ta);
		sjson::parse(platform, cfg[CROWN_PLATFORM_NAME]);

		if (json_object::has(platform, "renderer"))
		{
			JsonObject renderer(ta);
			sjson::parse(renderer, platform["renderer"]);

			if (json_object::has(renderer, "resolution"))
			{
				JsonArray resolution(ta);
				sjson::parse_array(resolution, renderer["resolution"]);
				window_w = sjson::parse_int(resolution[0]);
				window_h = sjson::parse_int(resolution[1]);
			}
			if (json_object::has(renderer, "aspect_ratio"))
				aspect_ratio = sjson::parse_float(renderer["aspect_ratio"]);
			if (json_object::has(renderer, "vsync"))
				vsync = sjson::parse_bool(renderer["vsync"]);
			if (json_object::has(renderer, "fullscreen"))
				fullscreen = sjson::parse_bool(renderer["fullscreen"]);
		}
	}

	return true;
}

} // namespace crown
