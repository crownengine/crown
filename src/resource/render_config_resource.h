/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/value.h"
#include "core/math/types.h"
#include "resource/types.h"
#include "core/containers/types.h"

namespace crown
{
struct RenderSettingsFlags
{
	enum Enum : u32
	{
		SUN_SHADOWS                     = u32(1) << 0, ///< Whether shadows for the sun are enabled.
		LOCAL_LIGHTS                    = u32(1) << 1, ///< Whether local lights are enabled.
		LOCAL_LIGHTS_SHADOWS            = u32(1) << 2, ///< Whether shadows for local lights are enabled.
		LOCAL_LIGHTS_DISTANCE_CULLING   = u32(1) << 3, ///< Whether distance culling for local lights is enabled.
		BLOOM                           = u32(1) << 4, ///< Whether bloom post-processing effect is enabled.
		MSAA                            = u32(1) << 5, ///< Whether multisample AA is enabled.
		OBJECT_CONTRIBUTION_CULLING     = u32(1) << 6, ///< Whether contribution culling for visible objects is enabled.
		SUN_SHADOW_CONTRIBUTION_CULLING = u32(1) << 7, ///< Whether contribution culling for sun shadows is enabled.
		SELECTION                       = u32(1) << 8  ///< Whether selection rendering is enabled.
	};
};

struct RenderSettings
{
	u32 flags;
	Vector2 sun_shadow_map_size;
	Vector2 local_lights_shadow_map_size;
	Vector4 shadow_map_params[2];             ///< Texel sizes followed by filtering sample counts.
	f32 sun_shadow_split_weight;              ///< Weight of logarithmic sun shadow split distribution.
	f32 sun_shadow_split_overlap;             ///< Fraction of a sun shadow split that overlaps the next.
	f32 sun_shadow_max_caster_distance;       ///< Maximum distance the caster region extends toward the sun.
	f32 object_contribution_culling_min_screen_size;     ///< Objects smaller than this in both projected dimensions are culled.
	f32 sun_shadow_contribution_culling_min_screen_size; ///< Shadow casters smaller than this in both projected dimensions are culled.
	f32 local_lights_distance_culling_fade;   ///< Distance from camera at which local lights start to fade.
	f32 local_lights_distance_culling_cutoff; ///< Distance from camera at which local lights disappear.
	f32 lod_fade_duration;                    ///< Duration in seconds of LOD crossfades.
	u32 msaa_quality;
	Vector2 lights_cookie_atlas_size; ///< Fixed total size of the shared light cookie atlas.
};

struct RenderConfigResource
{
	u32 version;
	RenderSettings render_settings;
};

namespace render_settings
{
	///
	s32 parse(HashMap<StringId32, Value> &rs, const char *settings_json);

	///
	s32 write(RenderSettings &rs, const HashMap<StringId32, Value> &settings_map);

} // namespace render_settings

} // namespace crown
