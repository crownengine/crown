/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "boot_config.h"
#include "dynamic_string.h"
#include "json_object.h"
#include "map.h"
#include "platform.h"
#include "sjson.h"
#include "temp_allocator.h"

namespace crown
{
BootConfig::BootConfig(Allocator& a)
	: boot_script_name(u64(0))
	, boot_package_name(u64(0))
	, window_title(a)
	, window_w(CROWN_DEFAULT_WINDOW_WIDTH)
	, window_h(CROWN_DEFAULT_WINDOW_HEIGHT)
	, aspect_ratio(-1.0f)
	, vsync(true)
{
}

bool BootConfig::parse(const char* json)
{
	TempAllocator4096 ta;
	JsonObject cfg(ta);
	sjson::parse(json, cfg);

	// General configs
	boot_script_name  = sjson::parse_resource_id(cfg["boot_script"]);
	boot_package_name = sjson::parse_resource_id(cfg["boot_package"]);

	if (json_object::has(cfg, "window_title"))
		sjson::parse_string(cfg["window_title"], window_title);

	// Platform-specific configs
	if (json_object::has(cfg, CROWN_PLATFORM_NAME))
	{
		JsonObject platform(ta);
		sjson::parse(cfg[CROWN_PLATFORM_NAME], platform);

		if (json_object::has(platform, "renderer"))
		{
			JsonObject renderer(ta);
			sjson::parse(platform["renderer"], renderer);

			if (json_object::has(renderer, "resolution"))
			{
				JsonArray resolution(ta);
				sjson::parse_array(renderer["resolution"], resolution);
				window_w = sjson::parse_int(resolution[0]);
				window_h = sjson::parse_int(resolution[1]);
			}
			if (json_object::has(renderer, "aspect_ratio"))
				aspect_ratio = sjson::parse_float(renderer["aspect_ratio"]);
			if (json_object::has(renderer, "vsync"))
				vsync = sjson::parse_bool(renderer["vsync"]);
		}
	}

	return true;
}

} // namespace crown
