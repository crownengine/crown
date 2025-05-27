/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "world/types.h"
#include "resource/shader_resource.h"
#include <bgfx/bgfx.h>

#define MAX_NUM_LIGHTS 32
#define MAX_NUM_SPRITE_LAYERS 8

struct View
{
	enum Enum
	{
		SPRITE_0,
		SPRITE_LAST = SPRITE_0 + MAX_NUM_SPRITE_LAYERS,
		LIGHTS      = SPRITE_LAST,
		MESH,
		WORLD_GUI,
		SELECTION,
		OUTLINE,
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

	// Main output color/depth buffers.
	bgfx::TextureHandle _main_color_texture;
	bgfx::TextureHandle _main_depth_texture;
	bgfx::FrameBufferHandle _main_frame_buffer;
	bgfx::UniformHandle _main_color_texture_sampler;
	bgfx::UniformHandle _main_depth_texture_sampler;

	// Selection id/depth buffers.
	bgfx::TextureHandle _selection_texture;
	bgfx::TextureHandle _selection_depth_texture;
	bgfx::FrameBufferHandle _selection_frame_buffer;
	bgfx::UniformHandle _selection_texture_sampler;
	bgfx::UniformHandle _selection_depth_texture_sampler;
	bgfx::UniformHandle _outline_color_uniform;

	// Default sampler/texture to keep WebGL renderer running
	// when a shader references a texture (even if unused) but
	// none are set via setTexture().
	bgfx::UniformHandle _html5_default_sampler;
	bgfx::TextureHandle _html5_default_texture;

	// Default shaders.
	ShaderData _blit_shader;
	ShaderData _gui_shader;
	ShaderData _gui_3d_shader;
	ShaderData _debug_line_depth_enabled_shader;
	ShaderData _debug_line_shader;
	ShaderData _outline_shader;
	ShaderData _selection_shader;

	///
	Pipeline(ShaderManager &sm);

	///
	void create(u16 width, u16 height);

	///
	void destroy();

	///
	void reset(u16 width, u16 height);

	///
	void render(u16 width, u16 height);

	///
	void reload_shaders(const ShaderResource *old_resource, const ShaderResource *new_resource);
};

} // namespace crown
