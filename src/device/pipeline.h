/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "world/types.h"
#include <bgfx/bgfx.h>

#define VIEW_SPRITE_0   1
#define VIEW_SPRITE_1   2
#define VIEW_SPRITE_2   3
#define VIEW_SPRITE_3   4
#define VIEW_SPRITE_4   5
#define VIEW_SPRITE_5   6
#define VIEW_SPRITE_6   7
#define VIEW_SPRITE_7   8
#define VIEW_MESH      16
#define VIEW_DEBUG     17
#define VIEW_GUI      128
#define VIEW_GRAPH    129
#define VIEW_BLIT     254
#define VIEW_IMGUI    255

namespace crown
{
struct Pipeline
{
	bgfx::TextureHandle _buffers[2];
	bgfx::FrameBufferHandle _frame_buffer;
	bgfx::UniformHandle _tex_color;

	///
	Pipeline();

	///
	void create(uint16_t width, uint16_t height);

	///
	void destroy();

	///
	void reset(u16 width, u16 height);

	///
	void render(ShaderManager& sm, StringId32 program, uint8_t view, uint16_t width, uint16_t height);
};

} // namespace crown
