/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/types.h"

namespace crown
{
namespace splash
{
	constexpr u32 BACKGROUND_COLOR = 0;
	constexpr u32 TRACK_COLOR = 0x00101010;
	constexpr u32 FOREGROUND_COLOR = 0x00f0f0f0;

	/// 1-bit monochrome or 32-bit BGRX pixels, with rows padded to 32 bits.
	struct Bitmap
	{
		const u8 *pixels;
		u32 width;
		u32 height;
		u32 bits_per_pixel;

		u32 stride() const
		{
			return (width*bits_per_pixel + 31)/32*4;
		}
	};

	extern const Bitmap logo;
	extern const Bitmap label;

	struct Rectangle
	{
		int x;
		int y;
		u32 width;
		u32 height;
		u32 color;
	};

	inline Rectangle progress_rectangle(u32 width, u32 height, f32 previous, f32 progress)
	{
		const u32 bar_width = min(480u, width*4/5);
		const u32 begin = u32(bar_width*previous);
		const u32 end = u32(bar_width*progress);

		return { int((width - bar_width)/2 + min(begin, end))
				 , (int(height) - 130)/2 + 128
				 , max(begin, end) - min(begin, end)
				 , 2
				 , end < begin ? TRACK_COLOR : FOREGROUND_COLOR
		};
	}

	template<typename Painter, typename Image>
	void draw(Painter &painter, Image &logo, Image &label, u32 width, u32 height, f32 progress)
	{
		if (width == 0 || height == 0)
			return;

		const int top = (int(height) - 130)/2;
		const int bar_width = min(480, int(width)*4/5);
		const int bar_x = (int(width) - bar_width)/2;
		const int bar_y = top + 128;

		painter.fill_rectangle(0, 0, width, height, BACKGROUND_COLOR);
		painter.put_image((int(width) - int(logo.width))/2, top, logo);
		if (CROWN_SPLASH_TEXT)
			painter.put_image((int(width) - int(label.width))/2, top + 88, label);
		painter.fill_rectangle(bar_x, bar_y, bar_width, 2, TRACK_COLOR);
		painter.fill_rectangle(bar_x, bar_y, u32(bar_width*progress), 2, FOREGROUND_COLOR);
	}

	/// Draws into tightly packed, top-down 32-bit BGRX pixels.
	void draw(u32 *pixels, u32 width, u32 height, f32 progress);

} // namespace splash

} // namespace crown
