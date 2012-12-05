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

#pragma once

#include "Types.h"
#include "Color4.h"
#include "Resource.h"
#include "Image.h"

namespace crown
{

/// Enumerates the type of a texture
enum TextureType
{
	TT_1D		= 0,
	TT_2D		= 1,
	TT_3D		= 2,
	TT_CUBEMAP	= 3,
	TT_COUNT
};

///
enum TextureMode
{
	TM_MODULATE	= 0,	// Multiplies texel color by the geometry color after lighting
	TM_REPLACE	= 1,	// Replaces the fragment color with the texel color
	TM_DECAL	= 2,	// WTF?
	TM_BLEND	= 3,	// Blends the texel color with a constant blending color
	TM_ADD		= 4,	// Adds the texel color to the fragment color
	TM_COUNT
};


/// Enumerates the hardware filter to use when applying a texture
enum TextureFilter
{
	TF_NEAREST		= 0,
	TF_LINEAR		= 1,
	TF_BILINEAR		= 2,
	TF_TRILINEAR	= 3,
	TF_ANISOTROPIC	= 4,
	TF_COUNT
};


/// Enumerates the wrapping mode to use when applying a texture
enum TextureWrap
{
	TW_REPEAT			= 0,
	TW_CLAMP			= 1,
	TW_CLAMP_TO_EDGE	= 2,
	TW_CLAMP_TO_BORDER	= 3,
	TW_COUNT
};

class Stream;

class TextureResource
{
public:

	static TextureResource*		load(Stream* stream);
	static void					unload(TextureResource* texture);

private:

	uint32_t					width;
	uint32_t					height;

	TextureMode					mode;
	TextureFilter				filter;
	TextureWrap					wrap;
	
	void*						data;
};

} // namespace crown

