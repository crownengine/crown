/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "font_resource.h"
#include "allocator.h"
#include "filesystem.h"
#include "string_utils.h"
#include "compile_options.h"
#include "sjson.h"
#include "map.h"

namespace crown
{
namespace font_resource
{
	void parse_glyph(const char* json, FontGlyphData& glyph)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		glyph.id        = sjson::parse_int(obj["id"]);
		glyph.x         = sjson::parse_float(obj["x"]);
		glyph.y         = sjson::parse_float(obj["y"]);
		glyph.width     = sjson::parse_float(obj["width"]);
		glyph.height    = sjson::parse_float(obj["height"]);
		glyph.x_offset  = sjson::parse_float(obj["x_offset"]);
		glyph.y_offset  = sjson::parse_float(obj["y_offset"]);
		glyph.x_advance = sjson::parse_float(obj["x_advance"]);
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonArray glyphs(ta);

		sjson::parse(buf, object);
		sjson::parse_array(object["glyphs"], glyphs);

		const u32 size       = sjson::parse_int(object["size"]);
		const u32 font_size  = sjson::parse_int(object["font_size"]);
		const u32 num_glyphs = array::size(glyphs);

		Array<FontGlyphData> m_glyphs(default_allocator());

		for (u32 i = 0; i < num_glyphs; ++i)
		{
			FontGlyphData data;
			parse_glyph(glyphs[i], data);
			array::push_back(m_glyphs, data);
		}

		// Write
		FontResource fr;
		fr.version      = RESOURCE_VERSION_FONT;
		fr.num_glyphs   = array::size(m_glyphs);
		fr.texture_size = size;
		fr.font_size    = font_size;

		opts.write(fr.version);
		opts.write(fr.num_glyphs);
		opts.write(fr.texture_size);
		opts.write(fr.font_size);

		for (u32 i = 0; i < array::size(m_glyphs); ++i)
		{
			opts.write(m_glyphs[i].id);
			opts.write(m_glyphs[i].x);
			opts.write(m_glyphs[i].y);
			opts.write(m_glyphs[i].width);
			opts.write(m_glyphs[i].height);
			opts.write(m_glyphs[i].x_offset);
			opts.write(m_glyphs[i].y_offset);
			opts.write(m_glyphs[i].x_advance);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const u32 file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(u32*)res == RESOURCE_VERSION_FONT, "Wrong version");
		return res;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	const FontGlyphData* get_glyph(const FontResource* fr, u32 cp)
	{
		CE_ASSERT(cp < fr->num_glyphs, "Index out of bounds");

		FontGlyphData* begin = (FontGlyphData*)&fr[1];

		for (u32 i = 0; i < fr->num_glyphs; ++i)
		{
			if (begin[i].id == cp)
				return &begin[i];
		}

		CE_FATAL("Glyph not found");
		return NULL;
	}
} // namespace font_resource
} // namespace crown
