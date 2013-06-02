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

namespace crown
{

enum FogMode
{
	FM_LINEAR	= 0,
	FM_EXP		= 1,
	FM_EXP2		= 2,
	FM_COUNT
};

enum CompareFunction
{
	CF_NEVER	= 0,
	CF_LESS		= 1,
	CF_EQUAL	= 2,
	CF_LEQUAL	= 3,
	CF_GREATER	= 4,
	CF_NOTEQUAL	= 5,
	CF_GEQUAL	= 6,
	CF_ALWAYS	= 7,
	CF_COUNT
};

enum ShadingType
{
	ST_FLAT		= 0,
	ST_SMOOTH	= 1,
	ST_COUNT
};

enum PolygonMode
{
	PM_POINT	= 0,
	PM_LINE		= 1,
	PM_FILL		= 2,
	PM_COUNT
};

enum FrontFace
{
	FF_CW		= 0,
	FF_CCW		= 1,
	FF_COUNT
};

enum BlendEquation
{
	BE_FUNC_ADD					= 0,
	BE_FUNC_SUBTRACT			= 1,
	BE_FUNC_REVERSE_SUBTRACT	= 2,
	BE_MIN						= 3,
	BE_MAX						= 4,
	BE_COUNT
};

enum BlendFunction
{
	BF_ZERO						= 0,
	BF_ONE						= 1,
	BF_SRC_COLOR				= 2,
	BF_ONE_MINUS_SRC_COLOR		= 3,
	BF_DST_COLOR				= 4,
	BF_ONE_MINUS_DST_COLOR		= 5,
	BF_SRC_ALPHA				= 6,
	BF_ONE_MINUS_SRC_ALPHA		= 7,
	BF_DST_ALPHA				= 8,
	BF_ONE_MINUS_DST_ALPHA		= 9,
	BF_CONSTANT_COLOR			= 10,
	BF_ONE_MINUS_CONSTANT_COLOR	= 11,
	BF_CONSTANT_ALPHA			= 12,
	BF_ONE_MINUS_CONSTANT_ALPHA	= 13,
	BF_SRC_ALPHA_SATURATE		= 14,
	BF_COUNT
};

} // namespace crown
