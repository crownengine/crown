/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/types.h"
#include "resource/types.h"

namespace crown
{
struct FontResource
{
	u32 version;
	u32 texture_size;
	u32 font_size;
	u32 num_glyphs;
};

struct GlyphData
{
	f32 x;
	f32 y;
	f32 width;
	f32 height;
	f32 x_offset;
	f32 y_offset;
	f32 x_advance;
};

typedef u32 CodePoint;

namespace font_resource
{
	/// Returns the glyph for the code point @a cp.
	const GlyphData *glyph(const FontResource *fr, CodePoint cp, const GlyphData *deffault);

} // namespace font_resource

} // namespace crown
