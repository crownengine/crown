/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/value.h"
#include "core/math/types.h"
#include "resource/types.h"
#include "core/containers/types.h"
#include "core/strings/string_id.h"

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
		SELECTION                       = u32(1) << 8, ///< Whether selection rendering is enabled.
		LIGHTS_COOKIE                   = u32(1) << 9  ///< Whether cookie textures for lights are enabled.
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

// All fields in the compiled pipeline are 32-bit scalars. Offsets are relative
// to RenderConfigResource; the runtime never writes into this resource.
#define RENDER_CONFIG_MAX_TARGETS 8
#define RENDER_CONFIG_INVALID UINT32_MAX

struct RenderResourceFlags
{
	enum Enum : u32
	{
		RENDER_TARGET = 1u << 0,
		CLAMP         = 1u << 1,
		POINT         = 1u << 2,
		ANISOTROPIC   = 1u << 3,
		COMPARE       = 1u << 4,
		MSAA          = 1u << 5
	};
};

struct RenderResourceFormat
{
	enum Enum
	{
		RGBA8, BGRA8, RGBA16F, RGBA32F, R32U,
		D16, D24, D24S8, D32F,
		COUNT
	};
};

struct RenderLayerSort
{
	enum Enum { DEFAULT, SEQUENTIAL, FRONT_TO_BACK, BACK_TO_FRONT, COUNT };
};

struct RenderLayerTransform
{
	enum Enum { NONE, CAMERA, ORTHO_UNIT, ORTHO_SCREEN, ORTHO_GRAPH, ORTHO_UNIT_FLIPPED, COUNT };
};

struct RenderClearFlags
{
	enum Enum { COLOR = 1, DEPTH = 2, STENCIL = 4 };
};

struct RenderCondition
{
	u32 required;
	u32 excluded;
};

struct RenderConditionName
{
	StringId32 name;
	u32 name_offset;
};

struct RenderResourceData
{
	StringId32 name;
	u32 name_offset;
	StringId32 size_source; // "backbuffer" or a size supplied by the application.
	u32 format;
	u32 flags;
	u32 width;              // Both zero for a relative/external size.
	u32 height;
	f32 w_scale;
	f32 h_scale;
	u32 count;              // Separate textures, halved at each successive index.
	RenderCondition condition;
	u32 has_initial_value;  // Optional one-pixel RGBA8 fallback texture.
	u32 initial_value;
};

struct RenderResourceRef
{
	u32 resource;
	u32 index;
};

struct RenderTargetData
{
	RenderResourceRef colors[RENDER_CONFIG_MAX_TARGETS];
	u32 num_colors;
	RenderResourceRef depth;
	u32 clear_flags;
	u32 clear_rgba;
	f32 clear_depth;
	u32 clear_stencil;
	u32 sort;
	u32 transform;
	u32 touch;
	u32 manual_rect; // A native producer supplies the sub-rect and camera.
};

// Geometry is supplied by a frame-local source, not inferred from layer names.
struct RenderDrawViewMode
{
	enum Enum { SINGLE, VIEWS, GROUPS, COUNT };
};

struct RenderDrawTypes
{
	enum Enum { MESH = 1, SPRITE = 2, PROCEDURAL = 4, ALL = 7 };
};

struct RenderDrawFilter
{
	enum Enum { ALL, SELECTED, SHADOW_CASTERS, COUNT };
};

struct RenderDrawData
{
	StringId32 type;           // Registered frame operation; empty means no draw.
	StringId32 source;         // Frame-local batch or upload source.
	StringId32 context;        // Shader routing context, independent of source.
	StringId32 parameters;     // Optional frame parameter set.
	StringId32 shader;         // Optional rigid/procedural program override.
	StringId32 skinned_shader; // Required for skinned batches when overriding.
	RenderResourceRef resource; // Upload destination, otherwise invalid.
	u32 view_mode;
	u32 source_index;
	u32 types;
	u32 filter;
	u32 first_input;
	u32 num_inputs;
	u32 first_uniform;
	u32 num_uniforms;
	u32 object_id_uniform;     // Name offset or RENDER_CONFIG_INVALID.
};

struct RenderLayerData
{
	StringId32 name;
	u32 name_offset;
	RenderCondition condition;
	RenderTargetData target;
	u32 count;     // Consecutive views for native producers, e.g. sprite layers.
	u32 generator; // RENDER_CONFIG_INVALID for a geometry layer.
	RenderDrawData draw;
};

struct RenderGeneratorData
{
	StringId32 name;
	u32 name_offset;
	u32 first_modifier;
	u32 num_modifiers;
};

struct RenderInputData
{
	RenderResourceRef texture;
	RenderResourceRef fallback; // Used when the primary resource is not allocated.
	u32 sampler_name_offset;
	u32 stage;
	u32 flags; // RENDER_CONFIG_INVALID preserves the texture's sampler flags.
};

struct RenderUniformData
{
	u32 name_offset;
	Vector4 value;
};

struct RenderModifierData
{
	StringId32 type;         // Registered native callback, or "fullscreen_pass".
	StringId32 shader;       // Program/permutation name, NOT a .shader resource path.
	u32 name_offset;         // Profiling name.
	u32 resource;            // Optional resource-array context for a native callback.
	RenderCondition condition;
	RenderTargetData target;
	u32 first_input;
	u32 num_inputs;
	u32 first_uniform;
	u32 num_uniforms;
	u32 pixel_size_uniform;  // Name offset or RENDER_CONFIG_INVALID.
};

struct RenderShaderLayerData
{
	StringId32 shader;
	StringId32 context;
	StringId32 program;
	StringId32 skinned_program;
	u32 layer;
	u32 index;
};

struct RenderConfigResource
{
	u32 version;
	RenderSettings render_settings;
	u32 num_conditions;
	u32 conditions_offset;
	u32 num_resources;
	u32 resources_offset;
	u32 num_layers;
	u32 layers_offset;
	u32 num_generators;
	u32 generators_offset;
	u32 num_modifiers;
	u32 modifiers_offset;
	u32 num_inputs;
	u32 inputs_offset;
	u32 num_uniforms;
	u32 uniforms_offset;
	u32 num_shader_layers;
	u32 shader_layers_offset;
	u32 names_size;
	u32 names_offset;
};

CE_STATIC_ASSERT(sizeof(RenderConfigResource) == sizeof(RenderSettings) + 19*sizeof(u32));
CE_STATIC_ASSERT(sizeof(RenderCondition) == 8);
CE_STATIC_ASSERT(sizeof(RenderConditionName) == 8);
CE_STATIC_ASSERT(sizeof(RenderResourceData) == 56);
CE_STATIC_ASSERT(sizeof(RenderResourceRef) == 8);
CE_STATIC_ASSERT(sizeof(RenderTargetData) == 108);
CE_STATIC_ASSERT(sizeof(RenderDrawData) == 68);
CE_STATIC_ASSERT(sizeof(RenderLayerData) == 200);
CE_STATIC_ASSERT(sizeof(RenderGeneratorData) == 16);
CE_STATIC_ASSERT(sizeof(RenderInputData) == 28);
CE_STATIC_ASSERT(sizeof(RenderUniformData) == 20);
CE_STATIC_ASSERT(sizeof(RenderModifierData) == 152);
CE_STATIC_ASSERT(sizeof(RenderShaderLayerData) == 24);

namespace render_config_resource
{
	/// Returns the named condition bit, or zero when not declared.
	u32 condition_mask(const RenderConfigResource *r, StringId32 name);

} // namespace render_config_resource

namespace render_settings
{
	///
	s32 parse(HashMap<StringId32, Value> &rs, const char *settings_json);

	///
	s32 write(RenderSettings &rs, const HashMap<StringId32, Value> &settings_map);

} // namespace render_settings

} // namespace crown
