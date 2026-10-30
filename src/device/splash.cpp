/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS
#include "device/splash.h"
#include <charset.h>
#include <string.h>

namespace crown
{
extern "C"
{
extern const unsigned char *splash_logo_pixels;
extern const unsigned int splash_logo_width;
extern const unsigned int splash_logo_height;
}

struct SplashLabel
{
	static constexpr char TEXT[] = "Initializing...";
	static constexpr u32 WIDTH = (sizeof(TEXT) - 1)*8;
	static constexpr u32 HEIGHT = 16;
	static constexpr u32 STRIDE = (WIDTH + 31)/32*4;

	u8 pixels[STRIDE*HEIGHT];

	SplashLabel()
		: pixels()
	{
		CE_UNUSED(vga8x8);

		for (u32 ch = 0; ch < sizeof(TEXT) - 1; ++ch) {
			for (u32 row = 0; row < HEIGHT; ++row)
				pixels[row*STRIDE + ch] = vga8x16[u8(TEXT[ch])*HEIGHT + row];
		}
	}
};

static const SplashLabel splash_label;

struct SplashImage
{
	u32 *pixels;
	u32 width;
	u32 height;

	void fill_rectangle(int x, int y, u32 w, u32 h, u32 color)
	{
		for (int row = max(0, y); row < min(int(height), y + int(h)); ++row) {
			for (int col = max(0, x); col < min(int(width), x + int(w)); ++col)
				pixels[row*width + col] = color;
		}
	}

	void put_image(int x, int y, const splash::Bitmap &image)
	{
		const int begin = max(0, -x);
		const int end = min(int(image.width), int(width) - x);

		if (begin >= end)
			return;

		const u32 stride = image.stride();

		if (image.bits_per_pixel == 1) {
			for (int row = max(0, -y); row < min(int(image.height), int(height) - y); ++row) {
				for (int col = begin; col < end; ++col) {
					if (image.pixels[row*stride + col/8] & (0x80 >> (col%8)))
						pixels[(y + row)*width + (x + col)] = splash::FOREGROUND_COLOR;
				}
			}
		} else {
			for (int row = max(0, -y); row < min(int(image.height), int(height) - y); ++row) {
				memcpy(pixels + (y + row)*width + (x + begin)
					, image.pixels + row*stride + begin*4
					, (end - begin)*4
					);
			}
		}
	}
};

namespace splash
{
	const Bitmap logo = { splash_logo_pixels, splash_logo_width, splash_logo_height, 32 };
	const Bitmap label = { splash_label.pixels, SplashLabel::WIDTH, SplashLabel::HEIGHT, 1 };

	void draw(u32 *pixels, u32 width, u32 height, f32 progress)
	{
		SplashImage image = { pixels, width, height };

		splash::draw(image, logo, label, width, height, progress);
	}

} // namespace splash

} // namespace crown

#endif // if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS
