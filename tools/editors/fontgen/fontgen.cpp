#include <cstdio>
#include <cstdlib>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "Image.h"
#include "TGAImageLoader.h"
#include "FileStream.h"
#include "List.h"

using namespace Crown;


typedef List<uint> CodePointRangeList;
CodePointRangeList mCodePointRangeList;
void AddCodePointRange(uint min, uint max);
Image* BuildFontImage(const char* ttfFont, ushort ttfSize, ushort ttfResolution);

int main(int argc, char** argv)
{
	Image* fontImage;

	if (argc == 4)
	{
		fontImage = BuildFontImage(argv[1], atoi(argv[2]), atoi(argv[3]));
	}

	if (fontImage == NULL)
	{
		return -1;
	}

	fontImage->ConvertToRGBA8();

	TGAImageLoader tgaLoader;

	tgaLoader.SaveFile(fontImage, Str(argv[1]) + Str(".tga"));

	delete fontImage;

	FileStream fileStream(SOM_READ, Str(argv[1]) + Str(".txt"));
	TextReader textReader(&fileStream);

	return 0;
}

Image* BuildFontImage(const char* ttfFont, ushort ttfSize, ushort ttfResolution)
{
	uint					mMaxTextHeight;
	uint					mMaxCharacterHeight;
	uint					mMaxCharacterWidth;

	FileStream* font = new FileStream(SOM_READ, ttfFont);

	if (font == NULL)
	{
		printf("Error: Unable to open file: %s\n", ttfFont);
		return NULL;
	}

	// Start reading from the beginning
	font->Seek(0, SM_SeekFromBegin);
	uchar* fontBuffer = new uchar[font->GetSize()];
	font->ReadDataBlock(fontBuffer, font->GetSize());

	FT_Library library;

	if (FT_Init_FreeType(&library))
	{
		printf("Error: Could not initialize FreeType library.");
		return NULL;
	}

	FT_Face face;
	uint error;
	error = FT_New_Memory_Face(library, fontBuffer, font->GetSize(), 0, &face);

	// Done with font stream
	delete font;

	if (error == FT_Err_Unknown_File_Format)
	{
		printf("Error: Unknown file format.");
		return NULL;
	}
	else if (error)
	{
		printf("Error: Could not load font file.");
		return NULL;
	}

	if ((face->face_flags & FT_FACE_FLAG_SCALABLE) == 0)
	{
		printf("Error: The font file is not scalable.");
		return NULL;
	}

	FT_Set_Char_Size(face, ttfSize << 6, ttfSize << 6, ttfResolution, ttfResolution);

	if (mCodePointRangeList.IsEmpty())
	{
		AddCodePointRange(32, 126); // ASCII printable characters
	}

	int maxGlyphWidth = 0;
	int maxGlyphHeight = 0;
	int maxHoriBearingY = 0;
	uint glyphCount = 0;

	// Find the biggest character
	// For each range
	CE_ASSERT(mCodePointRangeList.GetSize() % 2 == 0);
	for (int i = 0; i < mCodePointRangeList.GetSize(); i += 2)
	{
		uint j = mCodePointRangeList[i + 0];
		uint k = mCodePointRangeList[i + 1];

		// For each character in range
		for (; j <= k; j++)
		{
			FT_Load_Glyph(face, FT_Get_Char_Index(face, j), FT_LOAD_RENDER);
			FT_Bitmap& bitmap = face->glyph->bitmap;
			FT_Glyph_Metrics& metrics = face->glyph->metrics;

			if (maxGlyphWidth < (bitmap.width + (metrics.horiBearingX >> 6)))
			{
				maxGlyphWidth = (bitmap.width + (metrics.horiBearingX >> 6));
			}

			if (maxGlyphHeight < (2 * bitmap.rows - (metrics.horiBearingY >> 6)))
			{
				maxGlyphHeight =  (2 * bitmap.rows - (metrics.horiBearingY >> 6));
			}

			if (maxHoriBearingY < (metrics.horiBearingY >> 6))
			{
				maxHoriBearingY = (metrics.horiBearingY >> 6);
			}

			glyphCount++;
		}
	}

	mMaxCharacterHeight = maxGlyphHeight;
	mMaxCharacterWidth = maxGlyphWidth;

	// Additional spacing between characters to avoid artifacts
	uint glyphSpacing = 3;
	uint glyphPerRow = (uint)(Math::Ceil(Math::Sqrt((real)(glyphCount * maxGlyphHeight / maxGlyphWidth))));

	uint bufferWidth = glyphPerRow * (maxGlyphWidth + glyphSpacing);
	uint bufferHeight = (uint)Math::Ceil((float) glyphCount / glyphPerRow) * (maxGlyphHeight + glyphSpacing);

	bufferWidth = Math::NextPow2(bufferWidth);
	bufferHeight = Math::NextPow2(bufferHeight);

	uchar* buffer = new uchar[bufferWidth * bufferHeight * 2];

	uint advanceX = 0;
	uint advanceY = 0;
	uint totalAdvance = 0;
	uint glyphRowCount = 0;
	int glyphBaseline = maxGlyphHeight - maxHoriBearingY;

	for (uint i = 0; i < bufferWidth * bufferHeight * 2; i++)
	{
		buffer[i] = 0;
	}

	mMaxTextHeight = 0;

	FileStream fileStream(SOM_WRITE, Str(ttfFont) + Str(".txt"));
	TextWriter fileWriter(&fileStream);

	CE_ASSERT(mCodePointRangeList.GetSize() % 2 == 0);
	// For each range
	for (int i = 0; i < mCodePointRangeList.GetSize(); i += 2)
	{
		uint j = mCodePointRangeList[i + 0];
		uint k = mCodePointRangeList[i + 1];

		// For each character in range
		for (; j <= k; j++)
		{
			FT_Load_Glyph(face, FT_Get_Char_Index(face, j), FT_LOAD_RENDER);
			FT_Bitmap& bitmap = face->glyph->bitmap;
			FT_Glyph_Metrics& metrics = face->glyph->metrics;

			// If we have reached the maximum glyphs per row
			if (glyphRowCount == glyphPerRow)
			{
				glyphRowCount = 0;
				advanceX = 0;
				advanceY += maxGlyphHeight + glyphSpacing;
			}

			int horiz = (metrics.horiBearingY >> 6) - bitmap.rows;

			totalAdvance = advanceX + (metrics.horiBearingX >> 6) + (advanceY + glyphBaseline + horiz) * bufferWidth;

			// Glyph drawing
			for (int h = 0; h < bitmap.rows; h++)
			{
				for (int w = 0; w < bitmap.width; w++)
				{
					uchar value = bitmap.buffer[w + bitmap.pitch * (bitmap.rows - h -1)];
					buffer[2 * (w + h * bufferWidth + totalAdvance)] = 255;
					buffer[2 * (w + h * bufferWidth + totalAdvance) + 1] = value;
				}
			}

			float baseline = (float) (metrics.horiBearingY >> 6);

//			fileWriter.WriteFormatted("%d %f %f %f %f %f %f %f %f\n",
//				j,
//				(1.0f / (float) bufferWidth)	* (float) advanceX,
//				(1.0f / (float) bufferWidth)	* ((float) advanceX + maxGlyphWidth),
//				(1.0f / (float) bufferHeight)	* (float) advanceY,
//				(1.0f / (float) bufferHeight)	* ((float) advanceY + maxGlyphHeight),
//				(float) bitmap.width,
//				(float) bitmap.rows,
//				(float) (face->glyph->advance.x >> 6),
//				baseline);

			uint totalHeight = bitmap.rows + (int)Math::Abs(bitmap.rows - baseline);

			mMaxTextHeight = Math::Max<uint>(mMaxTextHeight, totalHeight);

			advanceX += maxGlyphWidth + glyphSpacing;

			glyphRowCount++;
		}
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	if (fontBuffer)
	{
		delete[] fontBuffer;
	}

	// Done with FreeType stuff

	Image* fontImage = new Image();

	fontImage->DestroyImage();
	fontImage->CreateImage(PF_LA_8, bufferWidth, bufferHeight, buffer);

	return fontImage;
}

void AddCodePointRange(uint min, uint max)
{
	CE_ASSERT(min <= max);

	mCodePointRangeList.Append(min);
	mCodePointRangeList.Append(max);
}

