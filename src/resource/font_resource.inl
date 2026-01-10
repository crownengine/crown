/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/font_resource.h"

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

} // namespace crown
