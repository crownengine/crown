/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "world/types.h"
#include "resource/render_config_resource.h"
#include "resource/shader_resource.h"
#include <bgfx/bgfx.h>

#define MAX_NUM_LIGHTS 32
#define MAX_NUM_SPRITE_LAYERS 8
#define MAX_NUM_CASCADES 4
#define CASCADED_SHADOW_MAP_SLOT 10

struct View
{
	enum Enum
	{
		SPRITE_0,
		SPRITE_LAST  = SPRITE_0 + MAX_NUM_SPRITE_LAYERS,
		CASCADE_0    = SPRITE_LAST,
		CASCADE_LAST = CASCADE_0 + MAX_NUM_CASCADES,
		LIGHTS       = CASCADE_LAST,
		MESH,
		WORLD_GUI,
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
	bgfx::TextureHandle _color_texture;
	bgfx::TextureHandle _depth_texture;
	bgfx::FrameBufferHandle _frame_buffer;
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

	// Shadow mapping.
	bgfx::TextureHandle _sun_shadow_map_texture;
	bgfx::FrameBufferHandle _sun_shadow_map_frame_buffer;
	bgfx::UniformHandle _u_cascaded_shadow_map;
	bgfx::UniformHandle _u_cascaded_texel_size;
	bgfx::UniformHandle _u_cascaded_lights;
	bgfx::UniformHandle _u_local_lights_params;

	// Default sampler/texture to keep WebGL renderer running
	// when a shader references a texture (even if unused) but
	// none are set via setTexture().
	bgfx::UniformHandle _html5_default_sampler;
	bgfx::TextureHandle _html5_default_texture;

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

	///
	Pipeline(ShaderManager &sm);

	///
	void create(u16 width, u16 height, const RenderSettings &render_settings);

	///
	void destroy();

	///
	void reset(u16 width, u16 height);

	///
	void render(u16 width, u16 height);

	///
	void reload_shaders(const ShaderResource *old_resource, const ShaderResource *new_resource);

	///
	void set_local_lights_params_uniform();
};

} // namespace crown
