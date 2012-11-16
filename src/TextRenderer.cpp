/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "TextRenderer.h"
#include <GL/glew.h>	// FIXME
#include "Renderer.h"
#include "Device.h"

namespace crown
{

//-----------------------------------------------------------------------------
TextRenderer::TextRenderer()
{
}

//-----------------------------------------------------------------------------
TextRenderer::~TextRenderer()
{
}

//-----------------------------------------------------------------------------
void TextRenderer::BeginDraw()
{
}

//-----------------------------------------------------------------------------
void TextRenderer::EndDraw()
{
}

//-----------------------------------------------------------------------------
void TextRenderer::Draw(const Str& string, int32_t x, int32_t y, Font* font)
{
	Renderer* renderer = GetDevice()->GetRenderer();
	Font* mFont = font;
	Texture* tex = font->GetTexture();

	renderer->_SetTexturing(0, true);
	renderer->_SetTexture(0, tex);
	renderer->_SetTextureMode(0, tex->GetMode(), tex->GetBlendColor());
	renderer->_SetTextureFilter(0, tex->GetFilter());

	Color4 mColor = Color4::BLACK;
	uint32_t mCharSpacing = 0;
	uint32_t mLineSpacing = 0;
	

	glColor4fv(mColor.to_float_ptr());
    glPushMatrix();
    glTranslatef((float)x, (float)y, 0.0f);
  
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
    for (uint32_t i = 0; i < string.GetLength(); i++)
    {
        Glyph glyph;
        float left, right, bottom, top, advance, dummy;
        uint32_t ttSizeX, ttSizeY;
  
        glyph = mFont->GetGlyph(string[i]);
  
        glyph.GetMetrics(left, right, bottom, top, dummy, dummy, advance, dummy);
        ttSizeX = (uint32_t)((right - left) * tex->GetWidth());
        ttSizeY = (uint32_t)((top - bottom) * tex->GetHeight());

        glBegin(GL_QUADS);
        glTexCoord2f(left, top);
        glVertex2i(0, 0);
        glTexCoord2f(left, bottom);
        glVertex2i(0, ttSizeY);
        glTexCoord2f(right, bottom);
        glVertex2i(ttSizeX, ttSizeY);
        glTexCoord2f(right, top);
        glVertex2i(ttSizeX, 0);
        glEnd();
  
        glTranslatef(advance + (float)mCharSpacing, 0, 0);
    }
  
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
	//glDisable(GL_BLEND);
}

//-----------------------------------------------------------------------------
void TextRenderer::GetStrDimensions(const Str& string, uint32_t start, uint32_t end, int32_t& width, int32_t& height)
{
//	if (mFont != NULL)
//	{
//		return;
//	}

//	height = 0;
//	width = 0;

//	//TODO: Should take baseline int32_to account

//	if (end == (uint32_t)-1)
//		end = string.GetLength();
//	for (uint32_t i = start; i < end; i++)
//	{
//		Glyph glyph;
//		float gheight, gwidth, dummy, advance, baseline;

//		glyph = mFont->GetGlyph(string[i]);

//		glyph.GetMetrics(dummy, dummy, dummy, dummy, gwidth, gheight, advance, baseline);

//		height = Math::Max(height, (int32_t)gheight) + (int32_t)Math::Abs(gheight - baseline);
//		if (i < string.GetLength() - 1)
//			width += (int32_t)advance;
//		else
//		{
//			width += (int32_t)gwidth;
//		}
//	}
}

//-----------------------------------------------------------------------------
int32_t TextRenderer::GetStrIndexFromDimensions(const Str& string, uint32_t start, const Point2& position, Point2& charPosition)
{
//	if (mFont == NULL)
//	{
//		return -1;
//	}

//	int32_t width = 0;

//	for (uint32_t i = start; i < string.GetLength(); i++)
//	{
//		Glyph glyph;
//		float gheight, gwidth, dummy, advance;

//		glyph = mFont->GetGlyph(string[i]);

//		glyph.GetMetrics(dummy, dummy, dummy, dummy, gwidth, gheight, advance, dummy);

//		if (i >= string.GetLength())
//			advance = gwidth;

//		if (width + advance / 2.0f >= position.x)
//		{
//			charPosition.x = width;
//			charPosition.y = 0;
//			return i;
//		}
//		width += (int32_t)advance;
//	}
//	charPosition.x = width;
//	charPosition.y = 0;
//	return string.GetLength();
}

//-----------------------------------------------------------------------------
int32_t TextRenderer::GetMaxTextHeight()
{
//	return mFont->_GetMaxTextHeight();
}

} // namespace crown

