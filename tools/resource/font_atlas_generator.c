/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#define CROWN_RECT_MARGIN 1 ///< Right and bottom glyph rect margin in pixels.

struct GlyphData
{
	int id;        ///< Codepoint.
	int x;         ///< X-position inside the atlas.
	int y;         ///< Y-position inside the atlas.
	int width;     ///< In pixels.
	int height;    ///< In pixels.
	int x_offset;  ///< In pixels.
	int y_offset;  ///< In pixels.
	int x_advance; ///< In pixels.
};

struct FontAtlas
{
	uint8_t *image_data; ///< 8-bits alpha channel.
	int atlas_size;      ///< In pixels.
	struct GlyphData *glyphs;
	int num_glyphs;
};

/// Frees the @a atlas returned from crown_font_atlas_generate().
void crown_font_atlas_free(struct FontAtlas *atlas)
{
	if (atlas != NULL) {
		free(atlas->image_data);
		free(atlas->glyphs);
	}

	free(atlas);
}

/// Returns a FontAtlas structure.
/// Call crown_font_atlas_free() when you are done with it.
void *crown_font_atlas_generate(const char *font_path, int font_size, int range_min, int range_max)
{
	FT_Library ft_library;
	if (FT_Init_FreeType(&ft_library))
		return NULL;

	FT_Face ft_face;
	if (FT_New_Face(ft_library, font_path, 0, &ft_face))
		goto err_done_freetype;

	if (FT_Set_Pixel_Sizes(ft_face, 0, font_size))
		goto err_done_face;

	struct FontAtlas *atlas = malloc(sizeof(*atlas));
	if (atlas == NULL)
		goto err_done_face;

	atlas->num_glyphs = range_max - range_min + 1;
	atlas->glyphs = malloc(sizeof(*atlas->glyphs) * atlas->num_glyphs);

	// Load and render all the glyphs.
	for (int codepoint = range_min; codepoint <= range_max; ++codepoint) {
		FT_UInt glyph_index = FT_Get_Char_Index(ft_face, codepoint);
		if (glyph_index == 0) // Glyph not available.
			continue;

		if (FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_RENDER))
			continue;

		FT_Bitmap *glyph_bitmap = &ft_face->glyph->bitmap;

		struct GlyphData *glyph = &atlas->glyphs[codepoint - range_min];
		glyph->id        = codepoint;
		glyph->x         = 0;
		glyph->y         = 0;
		glyph->width     = glyph_bitmap->width;
		glyph->height    = glyph_bitmap->rows;
		glyph->x_advance = ft_face->glyph->advance.x / 64.0f;
		glyph->x_offset  = ft_face->glyph->bitmap_left;
		glyph->y_offset  = ft_face->glyph->bitmap_top;
	}

	// Find the optimal size for the atlas.
	int atlas_width  = 32;
	int atlas_height = 32;
	struct stbrp_rect *rects = NULL;
	struct stbrp_node *nodes = NULL;
	bool all_packed;

	do {
		// Pack glyph rects into the atlas box.
		struct stbrp_context ctx;
		free(nodes);
		nodes = (stbrp_node *)malloc(sizeof(*nodes) * atlas_width);
		stbrp_init_target(&ctx, atlas_width, atlas_height, nodes, atlas_width);

		free(rects);
		rects = (stbrp_rect *)malloc(sizeof(*rects) * atlas->num_glyphs);
		for (int ii = 0; ii < atlas->num_glyphs; ++ii) {
			rects[ii].id = ii;
			rects[ii].w = CROWN_RECT_MARGIN + atlas->glyphs[ii].width;
			rects[ii].h = CROWN_RECT_MARGIN + atlas->glyphs[ii].height;
		}
		all_packed = true;
		stbrp_pack_rects(&ctx, rects, atlas->num_glyphs);

		if (atlas_width >= 1024)
			break;

		// Up-size the atlas box if some rects failed to pack.
		for (int ii = 0; ii < atlas->num_glyphs; ++ii) {
			if (rects[ii].was_packed == 0) {
				all_packed = false;
				atlas_width  *= 2;
				atlas_height *= 2;
				break;
			}
		}
	} while (!all_packed);

	// Allocate the atlas image.
	atlas->atlas_size = atlas_width;
	atlas->image_data = (uint8_t *)calloc(atlas_width * atlas_height, sizeof(*atlas->image_data));
	if (!atlas->image_data)
		goto err_free_atlas;

	// Pack the glyphs into the atlas image.
	for (int codepoint = range_min; codepoint <= range_max; ++codepoint) {
		FT_UInt glyph_index = FT_Get_Char_Index(ft_face, codepoint);
		if (glyph_index == 0) // Glyph not available.
			continue;

		FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_RENDER);
		FT_Bitmap *glyph_bitmap = &ft_face->glyph->bitmap;

		stbrp_rect *rect = &rects[codepoint - range_min];
		if (rect->was_packed == 0)
			continue;

		struct GlyphData *glyph = &atlas->glyphs[rect->id];
		glyph->x = rect->x;
		glyph->y = rect->y;

		for (int y = 0; y < glyph_bitmap->rows; ++y) {
			for (int x = 0; x < glyph_bitmap->width; ++x) {
				uint8_t intensity = glyph_bitmap->buffer[y * glyph_bitmap->pitch + x];
				atlas->image_data[(rect->y + y) * atlas_width + (rect->x + x)] = intensity;
			}
		}
	}

	free(rects);
	free(nodes);
	return atlas;

err_free_atlas:
	crown_font_atlas_free(atlas);
err_free_rects:
	free(rects);
	free(nodes);
err_done_face:
	FT_Done_Face(ft_face);
err_done_freetype:
	FT_Done_FreeType(ft_library);
	return NULL;
}
