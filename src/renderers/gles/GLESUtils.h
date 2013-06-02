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

<<<<<<< HEAD
#include <GLES2/gl2.h>
#include <cassert>

=======
#include <GLES/gl.h>
#include "Assert.h"
>>>>>>> master
#include "Texture.h"
#include "Material.h"
#include "PixelFormat.h"

namespace crown
{

/// OpenGL utilities for converting from wrapped names to GL names and vice-versa.
class GLES
{
public:

	static GLenum			compare_function(CompareFunction function);
	static GLenum			blend_function(BlendFunction function);
	static GLenum			blend_equation(BlendEquation equation);
	static GLenum			texture_wrap(TextureWrap wrap);
	static void				texture_filter(TextureFilter filter, GLint& minFilter, GLint& magFilter);
	static GLenum			pixel_format(PixelFormat format);
	static PixelFormat		pixel_format_from_gl_format(GLenum format);

private:

	static const GLenum		COMPARE_FUNCTION_TABLE[CF_COUNT];
	static const GLenum		BLEND_FUNCTION_TABLE[BF_COUNT];
	static const GLenum		BLEND_EQUATION_TABLE[BE_COUNT];
	static const GLenum		TEXTURE_WRAP_TABLE[TW_COUNT];
	static const GLenum		TEXTURE_MIN_FILTER_TABLE[TF_COUNT];
	static const GLenum		TEXTURE_MAG_FILTER_TABLE[TF_COUNT];

	// Disable construction
	GLES();
};

//-----------------------------------------------------------------------------
inline GLenum GLES::compare_function(CompareFunction function)
{
	CE_ASSERT(function < CF_COUNT);

	return COMPARE_FUNCTION_TABLE[function];
}

//-----------------------------------------------------------------------------
inline GLenum GLES::blend_function(BlendFunction function)
{
	CE_ASSERT(function < BF_COUNT);

	return BLEND_FUNCTION_TABLE[function];
}

//-----------------------------------------------------------------------------
inline GLenum GLES::blend_equation(BlendEquation equation)
{
<<<<<<< HEAD
<<<<<<< HEAD
	assert(equation < BE_COUNT);
=======
	ce_assert(mode < TM_COUNT);
>>>>>>> master
=======
	CE_ASSERT(mode < TM_COUNT);
>>>>>>> master

	return BLEND_EQUATION_TABLE[equation];
}

//-----------------------------------------------------------------------------
inline GLenum GLES::texture_wrap(TextureWrap wrap)
{
	CE_ASSERT(wrap < TW_COUNT);

	return TEXTURE_WRAP_TABLE[wrap];
}

//-----------------------------------------------------------------------------
inline void GLES::texture_filter(TextureFilter filter, GLint& minFilter, GLint& magFilter)
{
	CE_ASSERT(filter < TF_COUNT);

	minFilter = TEXTURE_MIN_FILTER_TABLE[filter];
	magFilter = TEXTURE_MAG_FILTER_TABLE[filter];
}

//-----------------------------------------------------------------------------
<<<<<<< HEAD
=======
inline GLenum GLES::fog_mode(FogMode mode)
{
	CE_ASSERT(mode < FM_COUNT);

	return FOG_MODE_TABLE[mode];
}

//-----------------------------------------------------------------------------
>>>>>>> master
inline GLenum GLES::pixel_format(PixelFormat format)
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
inline PixelFormat GLES::pixel_format_from_gl_format(GLenum format)
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

