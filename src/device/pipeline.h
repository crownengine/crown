/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "world/types.h"
#include "resource/render_config_resource.h"
#include "resource/shader_resource.h"
#include <bgfx/bgfx.h>

#define LIGHT_SIZE 22     // Size of a light in vec4 units.
#define MAX_NUM_LIGHTS 32 // Maximum number of lights per frame.
#define MAX_NUM_SPRITE_LAYERS 8
#define MAX_NUM_CASCADES 4
#define LIGHTS_DATA_SLOT 12
#define CASCADED_SHADOW_MAP_SLOT 10
#define LOCAL_LIGHTS_SHADOW_MAP_SLOT 11
#define LOCAL_LIGHTS_MAX_SHADOW_CASTERS 16 // Maximum number of local shadow-casting lights per frame.
CE_STATIC_ASSERT(LOCAL_LIGHTS_MAX_SHADOW_CASTERS <= MAX_NUM_LIGHTS);
#define LOCAL_LIGHTS_SM_MAX_VIEWS (LOCAL_LIGHTS_MAX_SHADOW_CASTERS * 4) // Worst case all omni casters.
#define BLOOM_MIPS 6

struct View
{
	enum Enum
	{
		COLOR_0,
		COLOR_1,
		CASCADE_CLEAR,
		CASCADE_0,
		CASCADE_LAST          = CASCADE_0 + MAX_NUM_CASCADES,
		SM_LOCAL_CLEAR        = CASCADE_LAST,
		SM_LOCAL_0,
		SM_LOCAL_LAST         = SM_LOCAL_0 + LOCAL_LIGHTS_SM_MAX_VIEWS,
		LIGHTS                = SM_LOCAL_LAST,
		MESH,
		BLOOM_COPY,
		BLOOM_DOWNSAMPLE_0,
		BLOOM_DOWNSAMPLE_LAST = BLOOM_DOWNSAMPLE_0 + BLOOM_MIPS - 1,
		BLOOM_UPSAMPLE_0,
		BLOOM_UPSAMPLE_LAST   = BLOOM_UPSAMPLE_0 + BLOOM_MIPS - 1,
		BLOOM_COMBINE,
		DUMMY_BLIT,
		TONEMAP,
		SPRITE_0,
		SPRITE_LAST           = SPRITE_0 + MAX_NUM_SPRITE_LAYERS,
		WORLD_GUI             = SPRITE_LAST,
		SELECTION,
		OUTLINE,
		OUTLINE_BLIT,
		DEBUG,
		SCREEN_GUI,
		GRAPH,
		BLIT,
		IMGUI,

		COUNT
	};
};

namespace crown
{
/// Render pipeline.
///
/// @ingroup Device
struct Pipeline
{
	ShaderManager *_shader_manager;
	RenderSettings _render_settings;

	// Main output color/depth handles.
	bgfx::TextureHandle _color_textures[2];
	bgfx::TextureHandle _depth_texture;
	bgfx::FrameBufferHandle _colors[2];
	bgfx::UniformHandle _color_map;
	bgfx::UniformHandle _depth_map;

	// Selection/outline handles.
	bgfx::TextureHandle _selection_texture;
	bgfx::TextureHandle _selection_depth_texture;
	bgfx::FrameBufferHandle _selection_frame_buffer;
	bgfx::UniformHandle _selection_map;
	bgfx::UniformHandle _selection_depth_map;
	bgfx::TextureHandle _outline_color_texture;
	bgfx::FrameBufferHandle _outline_frame_buffer;
	bgfx::UniformHandle _outline_color_map;
	bgfx::UniformHandle _outline_color;
	bgfx::UniformHandle _unit_id;

	// Cascaded shadow mapping.
	bgfx::TextureHandle _sun_shadow_map_texture;
	bgfx::FrameBufferHandle _sun_shadow_map_frame_buffer;
	bgfx::UniformHandle _u_cascaded_shadow_map;
	bgfx::UniformHandle _u_cascaded_lights;
	bgfx::UniformHandle _u_shadow_maps_texel_sizes;
	bgfx::TextureHandle _local_lights_shadow_map_texture;
	bgfx::FrameBufferHandle _local_lights_shadow_map_frame_buffer;
	bgfx::UniformHandle _u_local_lights_shadow_map;
	bgfx::UniformHandle _u_local_lights_params;
	u16 _local_lights_tile_size;
	u16 _local_lights_tile_cols;

	// Lighting.
	bgfx::UniformHandle _lights_num;
	bgfx::UniformHandle _lights_data;
	bgfx::TextureHandle _lights_data_texture;
	bgfx::UniformHandle _fog_data;
	bgfx::UniformHandle _lighting_params;

	// Bloom.
	bgfx::FrameBufferHandle _bloom_frame_buffers[BLOOM_MIPS];
	bgfx::UniformHandle _bloom_map;
	bgfx::UniformHandle _map_pixel_size;
	bgfx::UniformHandle _bloom_params;
	BloomDesc _bloom;

	// Tonemap.
	bgfx::UniformHandle _tonemap_type;
	TonemapDesc _tonemap;

	// Default sampler/texture to keep WebGL renderer running
	// when a shader references a texture (even if unused) but
	// none are set via setTexture().
	bgfx::UniformHandle _html5_default_sampler;
	bgfx::TextureHandle _html5_default_texture;

	bgfx::UniformHandle _u_uv_scale;
	bgfx::UniformHandle _u_uv_offset;

	// Default shaders.
	ShaderData _blit_shader;
	ShaderData _blit_blend_shader;
	ShaderData _gui_shader;
	ShaderData _gui_3d_shader;
	ShaderData _debug_line_depth_enabled_shader;
	ShaderData _debug_line_shader;
	ShaderData _outline_shader;
	ShaderData _selection_shader;
	ShaderData _shadow_shader;
	ShaderData _shadow_skinning_shader;
	ShaderData _skydome_shader;
	ShaderData _bloom_downsample_shader;
	ShaderData _bloom_upsample_shader;
	ShaderData _bloom_combine_shader;
	ShaderData _tonemap_shader;

	///
	Pipeline(ShaderManager &sm);

	///
	void create(u16 width, u16 height, const RenderSettings &render_settings);

	///
	void destroy();

	///
	void reset(u16 width, u16 height);

	///
	void render(u16 width, u16 height, const Matrix4x4 &view, const Matrix4x4 &proj);

	///
	void reload_shaders(const ShaderResource *old_resource, const ShaderResource *new_resource);

	///
	void set_local_lights_params_uniform();

	///
	void set_global_lighting_params(GlobalLightingDesc *global_lighting);
};

} // namespace crown
