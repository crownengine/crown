/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "font_resource.h"
#include "json_parser.h"
#include "allocator.h"
#include "filesystem.h"
#include "string_utils.h"
#include "compile_options.h"

namespace crown
{
namespace font_resource
{
	void parse_glyph(JSONElement e, FontGlyphData& glyph)
	{
		glyph.id        = e.key("id").to_int();
		glyph.x         = e.key("x").to_int();
		glyph.y         = e.key("y").to_int();
		glyph.width     = e.key("width").to_int();
		glyph.height    = e.key("height").to_int();
		glyph.x_offset  = e.key("x_offset").to_float();
		glyph.y_offset  = e.key("y_offset").to_float();
		glyph.x_advance = e.key("x_advance").to_float();
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);
		JSONParser json(buf);
		JSONElement root = json.root();

		Array<FontGlyphData> m_glyphs(default_allocator());
		JSONElement count = root.key("count");
		JSONElement size = root.key("size");
		JSONElement font_size = root.key("font_size");
		JSONElement glyphs = root.key("glyphs");

		uint32_t num_glyphs = count.to_int();

		for (uint32_t i = 0; i < num_glyphs; i++)
		{
			FontGlyphData data;
			parse_glyph(glyphs[i], data);
			array::push_back(m_glyphs, data);
		}

		FontResource fr;
		fr.version = FONT_VERSION;
		fr.num_glyphs = array::size(m_glyphs);
		fr.texture_size = size.to_int();
		fr.font_size = font_size.to_int();

		opts.write(fr.version);
		opts.write(fr.num_glyphs);
		opts.write(fr.texture_size);
		opts.write(fr.font_size);

		for (uint32_t i = 0; i < array::size(m_glyphs); i++)
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
		const uint32_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(uint32_t*)res == FONT_VERSION, "Wrong version");
		return res;
	}

	void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	uint32_t num_glyphs(const FontResource* fr)
	{
		return fr->num_glyphs;
	}

	uint32_t texture_size(const FontResource* fr)
	{
		return fr->texture_size;
	}

	uint32_t font_size(const FontResource* fr)
	{
		return fr->font_size;
	}

	const FontGlyphData* get_glyph(const FontResource* fr, uint32_t i)
	{
		CE_ASSERT(i < num_glyphs(fr), "Index out of bounds");

		FontGlyphData* begin = (FontGlyphData*)((char*)fr + sizeof(FontResource));

		for (uint32_t i = 0; i < num_glyphs(fr); i++)
		{
			if (begin[i].id == i)
				return &begin[i];
		}

		CE_FATAL("Glyph not found");
		return NULL;
	}
} // namespace font_resource
} // namespace crown
