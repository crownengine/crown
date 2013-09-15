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
	PF_L_8 = 0,				///< Luminance only, 8-bit
	PF_L_16 = 3,			///< Luminance only, 16-bit
	PF_L_32 = 14,			///< Luminance only, 32-bit integer
	PF_L_FLOAT_32 = 15,		///< Luminance only, 32-bit flaoting point

	PF_LA_8 = 4,			///< Luminance and alpha, 8-bit each
	PF_LA_16 = 16,			///< Luminance and alpha, 16-bit each
	PF_LA_32 = 26,			///< Luminance and alpha, 32-bit integer each
	PF_LA_FLOAT_32 = 27,	///< Luminance and alpha, 32-bit floating point each

	PF_AL_8 = 5,			///< Luminance and alpha, 8-bit each
	PF_AL_16 = 17,			///< Luminance and alpha, 16-bit each
	PF_AL_32 = 28,			///< Luminance and alpha, 32-bit integer each
	PF_AL_FLOAT_32 = 29,	///< Luminance and alpha, 32-bit floating point each

	PF_RGB_8 = 12,			///< RGB values, 8-bit each
	PF_RGB_16 = 24,			///< RGB values, 16-bit each
	PF_RGB_32 = 32,			///< RGB values, 32-bit integer each
	PF_RGB_FLOAT_32 = 33,	///< RGB values, 32-bit floating point each

	PF_RGB_3_3_2 = 1,		///< Packed 8-bit RGB values
	PF_RGB_5_6_5 = 6,		///< Packed 16-bit RGB values

	PF_BGR_8 = 13,			///< BGR values, 8-bit each
	PF_BGR_16 = 25,			///< BGR values, 16-bit each
	PF_BGR_32 = 34,			///< BGR values, 32-bit integer each
	PF_BGR_FLOAT_32 = 35,	///< BGR values, 32-bit floating point each

	PF_BGR_2_3_3 = 2,		///< Packed 8-bit BGR values
	PF_BGR_5_6_5 = 7,		///< Packed 16-bit BGR values

	PF_RGBA_8 = 18,			///< RGBA values, 8-bit each
	PF_RGBA_16 = 30,		///< RGBA values, 16-bit each
	PF_RGBA_32 = 36,		///< RGBA values, 32-bit integer each
	PF_RGBA_FLOAT_32 = 37,	///< RGBA values, 32-bit floating point each

	PF_RGBA_4_4_4_4 = 8,	///< Packed 16-bit RGBA values
	PF_RGBA_5_5_5_1 = 9,	///< Packed 16-bit RGBA values
	PF_RGBA_8_8_8_8 = 19,	///< Packed 32-bit RGBA values
	PF_RGBA_10_10_10_2 = 20,///< Packed 32-bit RGBA values

	PF_ABGR_8 = 21,			///< ABGR values, 8-bit each
	PF_ABGR_16 = 31,		///< ABGR values, 16-bit each
	PF_ABGR_32 = 38,		///< ABGR values, 32-bit integer each
	PF_ABGR_FLOAT_32 = 39,	///< ABGR values, 32-bit floating point each

	PF_ABGR_4_4_4_4 = 10,	///< Packed 16-bit ABGR values
	PF_ABGR_1_5_5_5 = 11,	///< Packed 16-bit ABGR values
	PF_ABGR_8_8_8_8 = 22,	///< Packed 32-bit ABGR values
	PF_ABGR_2_10_10_10 = 23,///< Packed 32-bit ABGR values

	PF_UNKNOWN = 40
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
