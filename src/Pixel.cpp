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

#include "Pixel.h"
#include "Types.h"

namespace Crown
{

uint Pixel::GetBitsPerPixel(PixelFormat format)
{
	switch (format)
	{
		case PF_L_8:
		case PF_RGB_3_3_2:
		case PF_BGR_2_3_3:
		{
			return 8;
		}
		case PF_L_16:
		case PF_LA_8:
		case PF_AL_8:
		case PF_RGB_5_6_5:
		case PF_BGR_5_6_5:
		case PF_RGBA_4_4_4_4:
		case PF_RGBA_5_5_5_1:
		case PF_ABGR_4_4_4_4:
		case PF_ABGR_1_5_5_5:
		{
			return 16;
		}
		case PF_RGB_8:
		case PF_BGR_8:
		{
			return 24;
		}
		case PF_L_32:
		case PF_L_FLOAT_32:
		case PF_LA_16:
		case PF_AL_16:
		case PF_RGBA_8:
		case PF_RGBA_8_8_8_8:
		case PF_RGBA_10_10_10_2:
		case PF_ABGR_8:
		case PF_ABGR_8_8_8_8:
		case PF_ABGR_2_10_10_10:
		{
			return 32;
		}
		case PF_RGB_16:
		case PF_BGR_16:
		{
			return 48;
		}
		case PF_LA_32:
		case PF_LA_FLOAT_32:
		case PF_AL_32:
		case PF_AL_FLOAT_32:
		case PF_RGBA_16:
		case PF_ABGR_16:
		{
			return 64;
		}
		case PF_RGB_32:
		case PF_RGB_FLOAT_32:
		case PF_BGR_32:
		case PF_BGR_FLOAT_32:
		{
			return 96;
		}
		case PF_RGBA_32:
		case PF_RGBA_FLOAT_32:
		case PF_ABGR_32:
		case PF_ABGR_FLOAT_32:
		{
			return 128;
		}
		case PF_UNKNOWN:
		{
			return 0;
		}
		default:
		{
			return 0;
		}
	}
}

uint Pixel::GetBytesPerPixel(PixelFormat format)
{
	return GetBitsPerPixel(format) / 8;
}

} // namespace Crown

