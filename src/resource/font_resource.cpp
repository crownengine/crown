/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/string.inl"
#include "resource/compile_options.inl"
#include "resource/font_resource.h"
#include "resource/types.h"
#include <algorithm>

namespace crown
{
namespace font_resource
{
	const GlyphData *glyph(const FontResource *fr, CodePoint cp, const GlyphData *deffault)
	{
		const CodePoint *codes  = (CodePoint *)&fr[1];
		const GlyphData *glyphs = (GlyphData *)(codes + fr->num_glyphs);

		// FIXME: Can do binary search
		for (u32 i = 0; i < fr->num_glyphs; ++i) {
			if (codes[i] == cp)
				return &glyphs[i];
		}

		return deffault;
	}

} // namespace font_resource

#if CROWN_CAN_COMPILE
namespace font_resource_internal
{
	struct GlyphInfo
	{
		CodePoint cp;
		GlyphData gd;

		bool operator<(const GlyphInfo &a) const
		{
			return cp < a.cp;
		}
	};

	s32 parse_glyphs(Array<GlyphInfo> &_glyphs, const JsonArray &glyphs, CompileOptions &opts)
	{
		for (u32 i = 0; i < array::size(glyphs); ++i) {
			TempAllocator512 ta;
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse(obj, glyphs[i]), opts);

			GlyphInfo gi;
			gi.cp           = RETURN_IF_ERROR(sjson::parse_int  (json_object::has(obj, "id") ? obj["id"] : obj["cp"]), opts);
			gi.gd.x         = RETURN_IF_ERROR(sjson::parse_float(obj["x"]), opts);
			gi.gd.y         = RETURN_IF_ERROR(sjson::parse_float(obj["y"]), opts);
			gi.gd.width     = RETURN_IF_ERROR(sjson::parse_float(obj["width"]), opts);
			gi.gd.height    = RETURN_IF_ERROR(sjson::parse_float(obj["height"]), opts);
			gi.gd.x_offset  = RETURN_IF_ERROR(sjson::parse_float(obj["x_offset"]), opts);
			gi.gd.y_offset  = RETURN_IF_ERROR(sjson::parse_float(obj["y_offset"]), opts);
			gi.gd.x_advance = RETURN_IF_ERROR(sjson::parse_float(obj["x_advance"]), opts);

			array::push_back(_glyphs, gi);
		}

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();

		TempAllocator4096 ta;
		JsonObject obj(ta);
		JsonArray glyphs(ta);

		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);
		RETURN_IF_ERROR(sjson::parse_array(glyphs, obj["glyphs"]), opts);

		const u32 texture_size = RETURN_IF_ERROR(sjson::parse_int(obj["size"]), opts);
		const u32 font_size    = RETURN_IF_ERROR(sjson::parse_int(obj["font_size"]), opts);
		RETURN_IF_FALSE(font_size > 0
			, opts
			, "Font size must be > 0"
			);

		s32 err = 0;
		Array<GlyphInfo> _glyphs(default_allocator());
		err = parse_glyphs(_glyphs, glyphs, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		std::sort(array::begin(_glyphs), array::end(_glyphs));

		opts.write(RESOURCE_HEADER(RESOURCE_VERSION_FONT));
		opts.write(texture_size);
		opts.write(font_size);
		opts.write(array::size(_glyphs));

		for (u32 i = 0; i < array::size(_glyphs); ++i)
			opts.write(_glyphs[i].cp);

		for (u32 i = 0; i < array::size(_glyphs); ++i) {
			opts.write(_glyphs[i].gd.x);
			opts.write(_glyphs[i].gd.y);
			opts.write(_glyphs[i].gd.width);
			opts.write(_glyphs[i].gd.height);
			opts.write(_glyphs[i].gd.x_offset);
			opts.write(_glyphs[i].gd.y_offset);
			opts.write(_glyphs[i].gd.x_advance);
		}

		return 0;
	}

} // namespace font_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
