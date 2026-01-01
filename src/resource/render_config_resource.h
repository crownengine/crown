/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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
		SUN_SHADOWS                   = u32(1) << 0, ///< Whether shadows for the sun are enabled.
		LOCAL_LIGHTS                  = u32(1) << 1, ///< Whether local lights are enabled.
		LOCAL_LIGHTS_SHADOWS          = u32(1) << 2, ///< Whether shadows for local lights are enabled.
		LOCAL_LIGHTS_DISTANCE_CULLING = u32(1) << 3, ///< Whether distance culling for local lights is enabled.
		BLOOM                         = u32(1) << 4, ///< Whether bloom post-processing effect is enabled.
		MSAA                          = u32(1) << 5  ///< Whether multisample AA is enabled.
	};
};

struct RenderSettings
{
	u32 flags;
	Vector2 sun_shadow_map_size;
	Vector2 local_lights_shadow_map_size;
	f32 local_lights_distance_culling_fade;   ///< Distance from camera at which local lights start to fade.
	f32 local_lights_distance_culling_cutoff; ///< Distance from camera at which local lights disappear.
	u32 msaa_quality;
};

struct RenderConfigResource
{
	u32 version;
	RenderSettings render_settings;
};

} // namespace crown
