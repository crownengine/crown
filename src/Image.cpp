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

#include "Image.h"
#include "Log.h"
#include "Types.h"
#include "Stream.h"

namespace Crown
{

Image::Image() :
	mPixelFormat(PF_UNKNOWN),
	mWidth(0),
	mHeight(0),
	mBuffer(NULL)
{
}

Image::Image(PixelFormat pixelFormat, uint width, uint height, uchar* data) :
	mPixelFormat(pixelFormat),
	mWidth(width),
	mHeight(height),
	mBuffer(data)
{
	if (data == NULL)
	{
		CreateBuffer();
		SetUniformColorImage(Color4::WHITE);
	}
}

Image::~Image()
{
	DestroyImage();
}

void Image::CreateImage(PixelFormat pixelFormat, uint width, uint height, uchar* data)
{
	mPixelFormat = pixelFormat;
	mWidth = width;
	mHeight = height;
	mBuffer = data;
}

void Image::DestroyImage()
{
	if (mBuffer)
	{
		delete[] mBuffer;
	}

	mPixelFormat = PF_UNKNOWN;
	mWidth = 0;
	mHeight = 0;
	mBuffer = NULL;
}

void Image::SetUniformColorImage(Color4 color)
{
	AssertRGB8();
	int bpp = GetBytesPerPixel();

	uchar red   = (uchar)(color.r * 255.0f);
	uchar green = (uchar)(color.g * 255.0f);
	uchar blue  = (uchar)(color.b * 255.0f);

	for(uint i = 0; i < mHeight; i++)
	{
		int rowOffset = i * mWidth * bpp;
		for(uint j = 0; j < mWidth; j++)
		{
			int offset = rowOffset + bpp * j;
			mBuffer[offset    ] = red;
			mBuffer[offset + 1] = green;
			mBuffer[offset + 2] = blue;
		}
	}
}

uint Image::GetWidth() const
{
	return mWidth;
}

uint Image::GetHeight() const
{
	return mHeight;
}

PixelFormat Image::GetFormat() const
{
	return mPixelFormat;
}

uint Image::GetBitsPerPixel() const
{
	return Pixel::GetBitsPerPixel(mPixelFormat);
}

uint Image::GetBytesPerPixel() const
{
	return Pixel::GetBytesPerPixel(mPixelFormat);
}

uchar* Image::GetBuffer()
{
	return mBuffer;
}

const uchar* Image::GetBuffer() const
{
	return mBuffer;
}

void Image::ApplyColorKeying(const Color4& color)
{
	assert(mPixelFormat == PF_RGBA_8);

	for (ulong i = 0; i < mWidth * mHeight * 4; i += 4)
	{
		if (Color4(mBuffer[i], mBuffer[i+1], mBuffer[i+2]) == color)
		{
			mBuffer[i] = 0;
			mBuffer[i+1] = 0;
			mBuffer[i+2] = 0;
			mBuffer[i+3] = 0;
		}
	}
}

void Image::ApplyGreyscaleToAlpha(Image* greyscaleImage)
{
	if (mPixelFormat != PF_RGBA_8)
	{
		Log::E("Image::ApplyGreyscaleToAlpha: Can apply alpha only on RGBA8 pixel formats.");
		return;
	}

	if (greyscaleImage == NULL)
	{
		Log::E("Image::ApplyGreyscaleToAlpha: greyscaleImage is NULL.");
		return;
	}

	if (greyscaleImage->mPixelFormat != PF_RGBA_8)
	{
		Log::E("Image::ApplyGreyscaleToAlpha: greyscaleImage must have pixel format RGBA8.");
		return;
	}

	if (mWidth != greyscaleImage->mWidth || mHeight != greyscaleImage->mHeight)
	{
		Log::E("Image::ApplyGreyscaleToAlpha: greyscaleImage must have the same dimensions of the image.");
		return;
	}

	for (ulong i = 0; i < mWidth * mHeight * 4; i += 4)
	{
		mBuffer[i+3] = greyscaleImage->mBuffer[i];
	}
}

void Image::AlphaToGreyscale()
{
	for (ulong i = 0; i < mWidth * mHeight * 4; i += 4)
	{
		mBuffer[i]   = mBuffer[i+3];
		mBuffer[i+1] = mBuffer[i+3];
		mBuffer[i+2] = mBuffer[i+3];
		mBuffer[i+3] = 255;
	}
}

void Image::CreateBuffer()
{
	if (mBuffer == NULL)
	{
		mBuffer = new uchar[mWidth * mHeight * GetBytesPerPixel()];
	}
}

void Image::AssertRGB8()
{
	assert(mPixelFormat == PF_RGB_8);
}

void Image::SetPixel(uint x, uint y, Color4 color)
{
	/*AssertRGB8();
	if (x >= mWidth || y >= mHeight)
	{
		throw ArgumentException("Coordinates outside the Image");
	}*/

	int bpp = 3;//GetBytesPerPixel();
	int offset = (y * mWidth + x) * GetBytesPerPixel();
	mBuffer[offset    ] = (uchar)(color.r * 255);
	mBuffer[offset + 1] = (uchar)(color.g * 255);
	mBuffer[offset + 2] = (uchar)(color.b * 255);
}

void Image::CopyTo(Image& dest) const
{
	dest.mWidth = mWidth;
	dest.mHeight = mHeight;
	dest.mPixelFormat = mPixelFormat;
	dest.CreateBuffer();

	for (ulong i = 0; i < mWidth * mHeight * GetBytesPerPixel(); i++)
	{
		dest.mBuffer[i] = mBuffer[i];
	}
}

void Image::ConvertToRGBA8()
{
	assert(mPixelFormat != PF_UNKNOWN);

	uchar* newBuf = new uchar[mWidth * mHeight * 4];

	uint j = 0;
	for (uint i = 0; i < mWidth * mHeight * 2; i += 2)
	{
		newBuf[j + 0] = mBuffer[i + 0];
		newBuf[j + 1] = mBuffer[i + 0];
		newBuf[j + 2] = mBuffer[i + 0];
		newBuf[j + 3] = mBuffer[i + 1];

		j += 4;
	}

	delete[] mBuffer;
	mBuffer = newBuf;
	mPixelFormat = PF_RGBA_8;
}

} // namespace Crown

