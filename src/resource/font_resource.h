/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"
#include "string_id.h"

namespace crown
{
struct FontResource
{
	u32 version;
	u32 num_glyphs;
	u32 texture_size; // Font texture size -- pow of 2
	u32 font_size;
};

struct FontGlyphData
{
	u32 id;
	f32 x;
	f32 y;
	f32 width;
	f32 height;
	f32 x_offset;
	f32 y_offset;
	f32 x_advance;
};

namespace font_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);

	const FontGlyphData* get_glyph(const FontResource* fr, u32 i);
} // namespace font_resource
} // namespace crown
