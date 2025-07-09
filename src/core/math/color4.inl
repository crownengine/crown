/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/types.h"

namespace crown
{
/// @addtogroup Math
/// @{

/// Returns a new color from individual elements. Alpha is set to 255.
inline Color4 from_rgb(u8 r, u8 g, u8 b)
{
	Color4 c;
	c.x = 1.0f/255.0f * r;
	c.y = 1.0f/255.0f * g;
	c.z = 1.0f/255.0f * b;
	c.w = 1.0f;
	return c;
}

/// Returns a new color from packed RGB integer.
inline Color4 from_rgb(u32 rgb)
{
	Color4 c;
	c.x = 1.0f/255.0f * ((rgb & 0xff0000) >> 16);
	c.y = 1.0f/255.0f * ((rgb & 0x00ff00) >> 8);
	c.z = 1.0f/255.0f * ((rgb & 0x0000ff) >> 0);
	c.w = 1.0f;
	return c;
}

/// Returns a new color from individual elements.
inline Color4 from_rgba(u8 r, u8 g, u8 b, u8 a)
{
	Color4 c;
	c.x = 1.0f/255.0f * r;
	c.y = 1.0f/255.0f * g;
	c.z = 1.0f/255.0f * b;
	c.w = 1.0f/255.0f * a;
	return c;
}

/// Returns a new color from packed RGBA integer.
inline Color4 from_rgba(u32 rgba)
{
	Color4 c;
	c.x = 1.0f/255.0f * ((rgba & 0xff000000) >> 24);
	c.y = 1.0f/255.0f * ((rgba & 0x00ff0000) >> 16);
	c.z = 1.0f/255.0f * ((rgba & 0x0000ff00) >> 8);
	c.w = 1.0f/255.0f * ((rgba & 0x000000ff) >> 0);
	return c;
}

/// Returns the color as a packed RGBA integer. Alpha is set to 255.
inline u32 to_rgb(const Color4 &c)
{
	u32 rgba;
	rgba  = u32(255.0f * c.x) << 24;
	rgba |= u32(255.0f * c.y) << 16;
	rgba |= u32(255.0f * c.z) << 8;
	rgba |= u32(255);
	return rgba;
}

/// Returns the color as a packed ABGR integer. Alpha is set to 255.
inline u32 to_bgr(const Color4 &c)
{
	u32 abgr;
	abgr  = u32(255) << 24;
	abgr |= u32(255.0f * c.z) << 16;
	abgr |= u32(255.0f * c.y) << 8;
	abgr |= u32(255.0f * c.x);
	return abgr;
}

/// Returns the color as a packed 32-bit integer. (RGBA order)
inline u32 to_rgba(const Color4 &c)
{
	u32 rgba;
	rgba  = u32(255.0f * c.x) << 24;
	rgba |= u32(255.0f * c.y) << 16;
	rgba |= u32(255.0f * c.z) << 8;
	rgba |= u32(255.0f * c.w);
	return rgba;
}

/// Returns the color as a packed 32-bit integer. (ABGR order)
inline u32 to_abgr(const Color4 &c)
{
	u32 abgr;
	abgr  = u32(255.0f * c.w) << 24;
	abgr |= u32(255.0f * c.z) << 16;
	abgr |= u32(255.0f * c.y) << 8;
	abgr |= u32(255.0f * c.x);
	return abgr;
}

/// @}

} // namespace crown
