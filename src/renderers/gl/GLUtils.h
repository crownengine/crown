/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#pragma once

#include <GL/glew.h>
#include "Texture.h"
#include "Material.h"
#include "PixelFormat.h"

namespace crown
{

/// OpenGL utilities for converting from wrapped names to GL names and vice-versa.
class GL
{
public:

	static GLenum			compare_function(CompareFunction function);
	static GLenum			blend_function(BlendFunction function);
	static GLenum			blend_equation(BlendEquation equation);
	static GLenum			texture_wrap(TextureWrap wrap);
	static void				texture_filter(TextureFilter filter, GLint& minFilter, GLint& magFilter);
	static GLenum			polygon_mode(PolygonMode mode);
	static GLenum			pixel_format(PixelFormat format);
	static PixelFormat		pixel_format_from_gl_format(GLenum format);

private:

	static const GLenum		COMPARE_FUNCTION_TABLE[CF_COUNT];
	static const GLenum		BLEND_FUNCTION_TABLE[BF_COUNT];
	static const GLenum		BLEND_EQUATION_TABLE[BE_COUNT];
	static const GLenum		TEXTURE_WRAP_TABLE[TW_COUNT];
	static const GLenum		TEXTURE_MIN_FILTER_TABLE[TF_COUNT];
	static const GLenum		TEXTURE_MAG_FILTER_TABLE[TF_COUNT];
	static const GLenum		POLYGON_MODE_TABLE[PM_COUNT];

	// Disable construction
	GL();
};

//-----------------------------------------------------------------------------
inline GLenum GL::compare_function(CompareFunction function)
{
	return COMPARE_FUNCTION_TABLE[function];
}

//-----------------------------------------------------------------------------
inline GLenum GL::blend_function(BlendFunction function)
{
	return BLEND_FUNCTION_TABLE[function];
}

//-----------------------------------------------------------------------------
inline GLenum GL::blend_equation(BlendEquation equation)
{
	return BLEND_EQUATION_TABLE[equation];
}

//-----------------------------------------------------------------------------
inline GLenum GL::texture_wrap(TextureWrap wrap)
{
	return TEXTURE_WRAP_TABLE[wrap];
}

//-----------------------------------------------------------------------------
inline void GL::texture_filter(TextureFilter filter, GLint& minFilter, GLint& magFilter)
{
	minFilter = TEXTURE_MIN_FILTER_TABLE[filter];
	magFilter = TEXTURE_MAG_FILTER_TABLE[filter];
}

//-----------------------------------------------------------------------------
inline GLenum GL::polygon_mode(PolygonMode mode)
{
	return POLYGON_MODE_TABLE[mode];
}

//-----------------------------------------------------------------------------
inline GLenum GL::pixel_format(PixelFormat format)
{
	switch (format)
	{
		case PF_RGBA_8:
			return GL_RGBA;
		case PF_RGB_8:
			return GL_RGB;
		case PF_LA_8:
			return GL_LUMINANCE_ALPHA;
		case PF_L_8:
			return GL_LUMINANCE;
		default:
			return GL_RGB;
	}
}

//-----------------------------------------------------------------------------
inline PixelFormat GL::pixel_format_from_gl_format(GLenum format)
{
	switch (format)
	{
		case GL_RGBA:
			return PF_RGBA_8;
		case GL_RGB:
			return PF_RGB_8;
		case GL_LUMINANCE_ALPHA:
			return PF_LA_8;
		case GL_LUMINANCE:
			return PF_L_8;
		default:
			return PF_RGB_8;
	}
}

} // namespace crown

