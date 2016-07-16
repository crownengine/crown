/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "dynamic_string.h"
#include "string_id.h"
#include "types.h"

namespace crown
{
/// Boot configuration.
///
/// @ingroup Device
struct BootConfig
{
	StringId64 boot_script_name;
	StringId64 boot_package_name;
	DynamicString window_title;
	u16 window_w;
	u16 window_h;
	float aspect_ratio;
	bool vsync;

	BootConfig(Allocator& a);
	bool parse(const char* json);
};

} // namespace crown
