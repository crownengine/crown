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

namespace crown
{

class Pixel
{
public:

	/// Returns the bytes occupied by @a format
	static size_t bytes_per_pixel(PixelFormat::Enum format)
	{
		switch (format)
		{
			case PixelFormat::RGB_8:
			{
				return 3;
			}
			case PixelFormat::RGBA_8:
			{
				return 4;
			}
			default:
			{
				CE_FATAL("Oops, unknown pixel format");
				return 0;
			}
		}
	}

	/// Returns the bits occupied by @a format
	static size_t bits_per_pixel(PixelFormat::Enum format)
	{
		return bytes_per_pixel(format) * 8;
	}

private:

	// Disable construction
	Pixel();
};

} // namespace crown

