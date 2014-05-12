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

#include "Types.h"

namespace crown
{

typedef Id VertexBufferId;
typedef Id IndexBufferId;
typedef Id RenderTargetId;
typedef Id TextureId;
typedef Id ShaderId;
typedef Id GPUProgramId;
typedef Id UniformId;

struct ShaderType
{
	enum Enum
	{
		VERTEX,
		FRAGMENT
	};
};

struct UniformType
{
	enum Enum
	{
		INTEGER_1,
		INTEGER_2,
		INTEGER_3,
		INTEGER_4,
		FLOAT_1,
		FLOAT_2,
		FLOAT_3,
		FLOAT_4,
		FLOAT_3x3,
		FLOAT_4x4,
		END
	};
};

struct ShaderUniform
{
	enum Enum
	{
		VIEW					= 0,
		MODEL					= 1,
		MODEL_VIEW				= 2,
		MODEL_VIEW_PROJECTION	= 3,
		TIME_SINCE_START		= 4,
		COUNT
	};
};

struct ShaderAttrib
{
	enum Enum
	{
		POSITION		= 0,
		NORMAL			= 1,
		COLOR			= 2,
		TEX_COORD0		= 3,
		TEX_COORD1		= 4,
		TEX_COORD2		= 5,
		TEX_COORD3		= 6,
		COUNT
	};
};

struct VertexFormat
{
	enum Enum
	{
		P2 = 0,
		P2_N3,
		P2_C4,
		P2_T2,
		P2_N3_C4,
		P2_N3_C4_T2,

		P3,
		P3_N3,
		P3_C4,
		P3_T2,
		P3_N3_C4,
		P3_N3_T2,
		P3_N3_C4_T2,

		COUNT
	};
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
struct PixelFormat
{
	enum Enum
	{
		DXT1,
		DXT3,
		DXT5,

		R8G8B8,
		R8G8B8A8,

		D16,
		D24,
		D32,
		D24S8,
		COUNT
	};
};

struct CommandType
{
	enum Enum
	{
		INIT_RENDERER,
		SHUTDOWN_RENDERER,

		CREATE_VERTEX_BUFFER,
		CREATE_DYNAMIC_VERTEX_BUFFER,
		UPDATE_VERTEX_BUFFER,
		DESTROY_VERTEX_BUFFER,

		CREATE_INDEX_BUFFER,
		CREATE_DYNAMIC_INDEX_BUFFER,
		UPDATE_INDEX_BUFFER,
		DESTROY_INDEX_BUFFER,

		CREATE_TEXTURE,
		UPDATE_TEXTURE,
		DESTROY_TEXTURE,

		CREATE_SHADER,
		DESTROY_SHADER,

		CREATE_GPU_PROGRAM,
		DESTROY_GPU_PROGRAM,

		CREATE_UNIFORM,
		DESTROY_UNIFORM,

		CREATE_RENDER_TARGET,
		DESTROY_RENDER_TARGET,

		END
	};
};

} // namespace crown
