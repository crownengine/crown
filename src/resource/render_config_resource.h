/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/types.h"
#include "resource/types.h"

namespace crown
{
struct RenderSettingsFlags
{
	enum Enum : u32
	{
		SUN_SHADOWS          = u32(1) << 0, ///< Whether shadows for the sun are enabled.
		LOCAL_LIGHTS_SHADOWS = u32(1) << 1  ///< Whether shadows for local lights are enabled.
	};
};

struct RenderSettings
{
	u32 flags;
	Vector2 sun_shadow_map_size;
	Vector2 local_lights_shadow_map_size;
};

struct RenderConfigResource
{
	u32 version;
	RenderSettings render_settings;
};

namespace render_config_resource_internal
{
	s32 compile(CompileOptions &opts);

} // namespace render_config_resource_internal

} // namespace crown
