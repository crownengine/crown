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
#include "RendererTypes.h"
#include "Assert.h"

namespace crown
{
namespace pixel_format
{

//-----------------------------------------------------------------------------
inline uint32_t size(PixelFormat::Enum fmt)
{
	switch (fmt)
	{
		case PixelFormat::DXT1: return 8;
		case PixelFormat::DXT3: return 16;
		case PixelFormat::DXT5: return 16;

		case PixelFormat::R8G8B8: return 3;
		case PixelFormat::R8G8B8A8: return 4;

		case PixelFormat::D16: return 2;
		case PixelFormat::D24: return 3;
		case PixelFormat::D32: return 4;
		case PixelFormat::D24S8: return 4;

		default: CE_FATAL("Unknown pixel format"); return 0;
	}
}

//-----------------------------------------------------------------------------
inline bool is_compressed(PixelFormat::Enum fmt)
{
	return fmt < PixelFormat::R8G8B8;
}

//-----------------------------------------------------------------------------
inline bool is_color(PixelFormat::Enum fmt)
{
	return fmt >= PixelFormat::R8G8B8 && fmt < PixelFormat::D16;
}

//-----------------------------------------------------------------------------
inline bool is_depth(PixelFormat::Enum fmt)
{
	return fmt >= PixelFormat::D16 && fmt < PixelFormat::COUNT;
}

} // namespace pixel_format
} // namespace crown
