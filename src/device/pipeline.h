/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "world/types.h"
#include "resource/shader_resource.h"
#include <bgfx/bgfx.h>

#define VIEW_SPRITE_0     1
#define VIEW_SPRITE_1     2
#define VIEW_SPRITE_2     3
#define VIEW_SPRITE_3     4
#define VIEW_SPRITE_4     5
#define VIEW_SPRITE_5     6
#define VIEW_SPRITE_6     7
#define VIEW_SPRITE_7     8
#define VIEW_MESH        16
#define VIEW_WORLD_GUI   17
#define VIEW_SELECTION   32
#define VIEW_OUTLINE     33
#define VIEW_DEBUG      100
#define VIEW_SCREEN_GUI 128
#define VIEW_GRAPH      200
#define VIEW_BLIT       254
#define VIEW_IMGUI      255

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
};

} // namespace crown
