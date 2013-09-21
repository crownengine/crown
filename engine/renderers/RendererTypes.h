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

#include "IdTable.h"

namespace crown
{

typedef Id VertexBufferId;
typedef Id IndexBufferId;
typedef Id RenderTargetId;
typedef Id TextureId;
typedef Id ShaderId;
typedef Id GPUProgramId;
typedef Id UniformId;

enum ShaderType
{
	SHADER_VERTEX,
	SHADER_FRAGMENT
};

enum UniformType
{
	UNIFORM_INTEGER_1,
	UNIFORM_INTEGER_2,
	UNIFORM_INTEGER_3,
	UNIFORM_INTEGER_4,
	UNIFORM_FLOAT_1,
	UNIFORM_FLOAT_2,
	UNIFORM_FLOAT_3,
	UNIFORM_FLOAT_4,
	UNIFORM_FLOAT_3_X_3,
	UNIFORM_FLOAT_4_X_4,
	UNIFORM_END
};

enum RenderTargetFormat
{
	RTF_RGB_8,		///< RGB values, 8-bit each
	RTF_RGBA_8,		///< RGBA values, 8-bit each
	RTF_D24			///< Depth
};

enum ShaderUniform
{
	UNIFORM_VIEW					= 0,
	UNIFORM_MODEL					= 1,
	UNIFORM_MODEL_VIEW				= 2,
	UNIFORM_MODEL_VIEW_PROJECTION	= 3,
	UNIFORM_TIME_SINCE_START		= 4,
	UNIFORM_COUNT
};

enum ShaderAttrib
{
	ATTRIB_POSITION			= 0,
	ATTRIB_NORMAL			= 1,
	ATTRIB_COLOR			= 2,
	ATTRIB_TEX_COORD0		= 3,
	ATTRIB_TEX_COORD1		= 4,
	ATTRIB_TEX_COORD2		= 5,
	ATTRIB_TEX_COORD3		= 6,
	ATTRIB_COUNT
};

enum VertexFormat
{
	VERTEX_P2 = 0,
	VERTEX_P2_N3,
	VERTEX_P2_C4,
	VERTEX_P2_T2,
	VERTEX_P2_N3_C4,
	VERTEX_P2_N3_C4_T2,

	VERTEX_P3,
	VERTEX_P3_N3,
	VERTEX_P3_C4,
	VERTEX_P3_T2,
	VERTEX_P3_N3_C4,
	VERTEX_P3_N3_T2,
	VERTEX_P3_N3_C4_T2,

	VERTEX_COUNT
};

// [0 - 2]		-> 8-bit
// [3 - 11] 	-> 16-bit
// [12 - 13]	-> 24-bit
// [14 - 23]	-> 32-bit
// [24 - 25]	-> 48-bit
// [26 - 31]	-> 64-bit
// [32 - 35]	-> 96-bit
// [36 - 39]	-> 128-bit
// 40			-> Unknown (0-bit)
/// Enumerates pixel formats.
enum PixelFormat
{
	PIXEL_RGB_8 = 0,
	PIXEL_RGBA_8,

	PIXEL_COUNT
};

enum CommandType
{
	COMMAND_INIT_RENDERER,
	COMMAND_SHUTDOWN_RENDERER,

	COMMAND_CREATE_VERTEX_BUFFER,
	COMMAND_CREATE_DYNAMIC_VERTEX_BUFFER,
	COMMAND_UPDATE_VERTEX_BUFFER,
	COMMAND_DESTROY_VERTEX_BUFFER,

	COMMAND_CREATE_INDEX_BUFFER,
	COMMAND_CREATE_DYNAMIC_INDEX_BUFFER,
	COMMAND_UPDATE_INDEX_BUFFER,
	COMMAND_DESTROY_INDEX_BUFFER,

	COMMAND_CREATE_TEXTURE,
	COMMAND_UPDATE_TEXTURE,
	COMMAND_DESTROY_TEXTURE,

	COMMAND_CREATE_SHADER,
	COMMAND_DESTROY_SHADER,

	COMMAND_CREATE_GPU_PROGRAM,
	COMMAND_DESTROY_GPU_PROGRAM,

	COMMAND_CREATE_UNIFORM,
	COMMAND_DESTROY_UNIFORM,

	COMMAND_END
};

} // namespace crown
