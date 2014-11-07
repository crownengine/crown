/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"

namespace crown
{

struct FontResource
{
	uint32_t version;
	uint32_t num_glyphs;
	uint32_t texture_size; // Font texture size -- pow of 2
	uint32_t font_size;
};

struct FontGlyphData
{
	uint32_t id;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	float x_offset;
	float y_offset;
	float x_advance;
};

namespace font_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);

	uint32_t num_glyphs(const FontResource* fr);
	uint32_t texture_size(const FontResource* fr);
	uint32_t font_size(const FontResource* fr);
	const FontGlyphData* get_glyph(const FontResource* fr, uint32_t i);
} // namespace font_resource
} // namespace crown
