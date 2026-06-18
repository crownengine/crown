/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/strings/dynamic_string.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "core/value.h"
#include "world/physics.h"

namespace crown
{
struct RendererType
{
	enum Enum
	{
		AUTO,
		DIRECT3D11,
		OPENGL,
		OPENGLES,
		VULKAN,

		COUNT
	};
};

/// Returns the renderer type named @a name or RendererType::COUNT if invalid.
RendererType::Enum renderer_type_from_name(const char *name);

/// Boot configuration.
///
/// @ingroup Device
struct BootConfig
{
	DynamicString boot_script_name;
	StringId64 boot_package_name;
	StringId64 render_config_name;
	StringId64 stat_config_name;
	DynamicString window_title;
	DynamicString save_dir;
	DynamicString user_config;
	RendererType::Enum renderer_type;
	u16 window_w;
	u16 window_h;
	u16 device_id;
	float aspect_ratio;
	bool vsync;
	bool fullscreen;
	PhysicsSettings physics_settings;
	HashMap<StringId32, Value> render_settings;

	///
	explicit BootConfig(Allocator &a);

	///
	bool parse(const char *json);
};

} // namespace crown
