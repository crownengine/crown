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
#include <algorithm>

namespace crown
{
namespace font_resource
{
	struct GlyphInfo
	{
		CodePoint cp;
		GlyphData gd;

		bool operator<(const GlyphInfo& a)
		{
			return cp < a.cp;
		}
	};

	void parse_glyph(const char* json, GlyphInfo& glyph)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		sjson::parse(json, obj);

		glyph.cp           = sjson::parse_int(obj["id"]);
		glyph.gd.x         = sjson::parse_float(obj["x"]);
		glyph.gd.y         = sjson::parse_float(obj["y"]);
		glyph.gd.width     = sjson::parse_float(obj["width"]);
		glyph.gd.height    = sjson::parse_float(obj["height"]);
		glyph.gd.x_offset  = sjson::parse_float(obj["x_offset"]);
		glyph.gd.y_offset  = sjson::parse_float(obj["y_offset"]);
		glyph.gd.x_advance = sjson::parse_float(obj["x_advance"]);
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonArray glyphs(ta);

		sjson::parse(buf, object);
		sjson::parse_array(object["glyphs"], glyphs);

		const u32 texture_size = sjson::parse_int(object["size"]);
		const u32 font_size    = sjson::parse_int(object["font_size"]);
		const u32 num_glyphs   = array::size(glyphs);

		Array<GlyphInfo> _glyphs(default_allocator());
		array::resize(_glyphs, num_glyphs);

		for (u32 i = 0; i < num_glyphs; ++i)
		{
			parse_glyph(glyphs[i], _glyphs[i]);
		}

		std::sort(array::begin(_glyphs), array::end(_glyphs));

		opts.write(RESOURCE_VERSION_FONT);
		opts.write(num_glyphs);
		opts.write(texture_size);
		opts.write(font_size);

		for (u32 i = 0; i < array::size(_glyphs); ++i)
		{
			opts.write(_glyphs[i].cp);
		}

		for (u32 i = 0; i < array::size(_glyphs); ++i)
		{
			opts.write(_glyphs[i].gd.x);
			opts.write(_glyphs[i].gd.y);
			opts.write(_glyphs[i].gd.width);
			opts.write(_glyphs[i].gd.height);
			opts.write(_glyphs[i].gd.x_offset);
			opts.write(_glyphs[i].gd.y_offset);
			opts.write(_glyphs[i].gd.x_advance);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const u32 size = file.size();
		void* res = a.allocate(size);
		file.read(res, size);
		CE_ASSERT(*(u32*)res == RESOURCE_VERSION_FONT, "Wrong version");
		return res;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	const GlyphData* get_glyph(const FontResource* fr, CodePoint cp)
	{
		CE_ASSERT(cp < fr->num_glyphs, "Index out of bounds");

		const CodePoint* pts  = (CodePoint*)&fr[1];
		const GlyphData* data = (GlyphData*)(pts + fr->num_glyphs);

		// FIXME: Can do binary search
		for (u32 i = 0; i < fr->num_glyphs; ++i)
		{
			if (pts[i] == cp)
				return &data[i];
		}

		CE_FATAL("Glyph not found");
		return NULL;
	}
} // namespace font_resource
} // namespace crown
