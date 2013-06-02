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

#include "GLUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
const GLenum GL::COMPARE_FUNCTION_TABLE[CF_COUNT] =
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

//-----------------------------------------------------------------------------
const GLenum GL::BLEND_FUNCTION_TABLE[BF_COUNT] =
{
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_CONSTANT_COLOR,
	GL_ONE_MINUS_CONSTANT_COLOR,
	GL_CONSTANT_ALPHA,
	GL_ONE_MINUS_CONSTANT_ALPHA,
	GL_SRC_ALPHA_SATURATE
};

//-----------------------------------------------------------------------------
const GLenum GL::BLEND_EQUATION_TABLE[BE_COUNT] =
{
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX
};

//-----------------------------------------------------------------------------
const GLenum GL::TEXTURE_WRAP_TABLE[TW_COUNT] =
{
	GL_REPEAT,
	GL_CLAMP,
	GL_CLAMP_TO_EDGE,
	GL_CLAMP_TO_BORDER
};

//-----------------------------------------------------------------------------
const GLenum GL::TEXTURE_MIN_FILTER_TABLE[TF_COUNT] =
{
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
};

//-----------------------------------------------------------------------------
const GLenum GL::TEXTURE_MAG_FILTER_TABLE[TF_COUNT] =
{
	GL_NEAREST,
	GL_LINEAR,
	GL_LINEAR,
	GL_LINEAR,
	GL_LINEAR
};

//-----------------------------------------------------------------------------
const GLenum GL::POLYGON_MODE_TABLE[PM_COUNT] =
{
	GL_POINT,
	GL_LINE,
	GL_FILL
};

} // namespace crown

