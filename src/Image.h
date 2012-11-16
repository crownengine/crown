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
#include "Pixel.h"

namespace crown
{

//! Holds an image
class Image
{

public:

	//! Constructor
	Image();

	//! Constructor
	Image(PixelFormat pixelFormat, uint32_t width, uint32_t height, uint8_t* data);

	//! Destructor
	~Image();

	//! Creates the image
	void CreateImage(PixelFormat pixelFormat, uint32_t width, uint32_t height, uint8_t* data);

	//! Destroys the image
	void DestroyImage();

	//! Returns the image's width in pixels
	uint32_t GetWidth() const;

	//! Returns the image's height in pixels
	uint32_t GetHeight() const;

	//! Returns the image's pixel format
	PixelFormat GetFormat() const;

	//! Returns the image's bits per pixel
	uint32_t GetBitsPerPixel() const;

	//! Returns the image's bytes per pixel
	uint32_t GetBytesPerPixel() const;

	//! Returns a point32_ter to the image data
	uint8_t* GetBuffer();

	//! Returns a const point32_ter to the image data
	const uint8_t* GetBuffer() const;

	//! Apply a color-keying
	void ApplyColorKeying(const Color4& color);

	//! Apply a greyscale image as alpha
	void ApplyGreyscaleToAlpha(Image* greyscaleImage);

	//! Use the alpha channel to build a greyscale opaque image
	void AlphaToGreyscale();

	//! Create a colored image of the same size of the current one
	void SetUniformColorImage(Color4 color);

	//! Set the pixel at location x,y to the specified color
	void SetPixel(uint32_t x, uint32_t y, Color4 color);

	void ConvertToRGBA8();

	/**
		Copies the image to dest.
	@note
		Destination image must be empty.
	*/
	void CopyTo(Image& dest) const;

private:

	PixelFormat	mPixelFormat;
	uint32_t		mWidth;
	uint32_t		mHeight;

	uint8_t*		mBuffer;

	void CreateBuffer();
	void AssertRGB8();

	// Disable copying
	Image(const Image&);
	Image& operator=(const Image&);
};

} // namespace crown

