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

#include <cstdio>
#include "Image.h"
#include "Log.h"
#include "Types.h"
#include "Str.h"
#include "BMPImageLoader.h"
#include "Device.h"
#include "Exceptions.h"
#include "Filesystem.h"

namespace crown
{

#pragma pack(2)

struct BitmapInfoHeader
{
	uint32_t biSize;
	int32_t  biWidth;
	int32_t  biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t  biXPelsPerMeter;
	int32_t  biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

struct BitmapFileHeader
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
};

#pragma pack()

BMPImageLoader::BMPImageLoader()
{
}

BMPImageLoader::~BMPImageLoader()
{
}

Image* BMPImageLoader::LoadFile(const char* relativePath)
{
	Image* image = NULL;
	Stream* fileStream;

	BitmapFileHeader bfh;
	BitmapInfoHeader bih;
	int32_t magicNumber = 0;
	int32_t padSize = 0;
	int32_t dataSize = 0;
	uint8_t* data = NULL;
	uint8_t* tmpdata = NULL;
	//<fp = fopen(name, "rb");
	fileStream = GetFilesystem()->OpenStream(relativePath, SOM_READ);

	if (fileStream == NULL)
	{
		return NULL;
	}

	fileStream->read(&magicNumber, 2);

	if (magicNumber != 19778)
	{
		Str msg = Str("BMPImageLoader::LoadFile: file '") + Str(relativePath) + Str("' is not a valid bitmap file.");
		Log::E(msg.c_str());
		//throw ArgumentException(msg.c_str());
		return NULL;
	}

	fileStream->seek(0);
	fileStream->read(&bfh, sizeof(BitmapFileHeader));
	fileStream->read(&bih, sizeof(BitmapInfoHeader));

	int32_t bpp = (bih.biBitCount/8);

	if (bpp != 3 && bpp != 4)
	{
		Str msg = "BMPImageLoader::LoadFile: Only 24bit and 32bit bitmaps are supported.";
		Log::E(msg.c_str());
		//throw ArgumentException(msg.c_str());
		throw (void*)NULL;
	}

	padSize = 0;

	while (((bih.biWidth*3+padSize) % 4) != 0)
	{
		padSize++;
	}

	dataSize = bih.biWidth * bih.biHeight * 4;
	data = new uint8_t[dataSize];

	if (bpp == 3)
	{
		tmpdata = new uint8_t[bih.biWidth*3];

		for (int32_t i=0; i<bih.biHeight ; i++)
		{
			fileStream->read(tmpdata, bih.biWidth*3);

			int32_t offset = bih.biWidth * 4 * i;

			for (int32_t j=0; j<bih.biWidth; j++)
			{
				data[offset++] = tmpdata[j*3+2];
				data[offset++] = tmpdata[j*3+1];
				data[offset++] = tmpdata[j*3+0];
				data[offset++] = 255;
			}

			if (padSize)
			{
				fileStream->skip(padSize);
			}
		}

		delete[] tmpdata;
	}
	else
	{
		for (int32_t i=0; i<bih.biHeight ; i++)
		{
			int32_t offset = bih.biWidth * 4 * i;

			fileStream->read(&data[offset], bih.biWidth*4);

			if (padSize)
			{
				fileStream->skip(padSize);
			}
		}
	}

	GetFilesystem()->Close(fileStream);

	image = new Image(PF_RGBA_8, bih.biWidth, bih.biHeight, data);
	return image;
}

void BMPImageLoader::SaveFile(const Image* image, const char* relativePath)
{
	FILE* fp = NULL;

	try
	{
		BitmapFileHeader bfh;
		BitmapInfoHeader bih;
		int32_t padSize = 0;
		int32_t imgDataSize = 0;
		int32_t bmpDataSize;
		const uint8_t* imgData = NULL;
		uint8_t* bmpRow = NULL;

		PixelFormat format = image->GetFormat();
		if (format != PF_LA_8 && format != PF_RGB_8 && format != PF_RGBA_8)
			throw 2;

		fp = fopen(relativePath, "wb");

		if (!fp)
		{
			throw 0;
		}

		while (( (image->GetWidth() * 3 + padSize) % 4) != 0) padSize++;;
		bmpDataSize = (image->GetWidth() * 3 + padSize) * image->GetHeight();

		bfh.bfType = 19778;
		bfh.bfSize = sizeof(bfh) + sizeof(bih) + bmpDataSize;
		bfh.bfOffBits = sizeof(bfh) + sizeof(bih);
		bfh.bfReserved1 = 0;
		bfh.bfReserved2 = 0;
		//compila la bih
		bih.biSize = sizeof(bih);
		bih.biWidth = image->GetWidth();
		bih.biHeight = image->GetHeight();
		bih.biPlanes = 1;
		bih.biBitCount = 24;
		bih.biCompression = 0;
		bih.biSizeImage = bmpDataSize;
		bih.biXPelsPerMeter = bih.biYPelsPerMeter = 0; //2835;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		fwrite(&bfh, sizeof(bfh), 1, fp);
		fwrite(&bih, sizeof(bih), 1, fp);


		int32_t bpp = image->GetBytesPerPixel();

		imgData = image->GetBuffer();
		imgDataSize = image->GetWidth() * image->GetHeight() * bpp;

		bmpRow = new uint8_t[bih.biWidth*3];

		for (int32_t i=0; i<bih.biHeight ; i++)
		{
			int32_t offset = bih.biWidth * bpp * i;

			for (int32_t j=0; j<bih.biWidth; j++)
			{
				if (bpp == 2)
				{
					bmpRow[j*3+2] = imgData[offset];
					bmpRow[j*3+1] = imgData[offset];
					bmpRow[j*3+0] = imgData[offset];
					offset += 2;
				}
				else
				{
					bmpRow[j*3+2] = imgData[offset++];
					bmpRow[j*3+1] = imgData[offset++];
					bmpRow[j*3+0] = imgData[offset++];
					if (bpp == 4)
						offset++;
				}
			}

			if (fwrite(bmpRow, 1, bih.biWidth*3, fp) != (uint32_t)bih.biWidth*3)
			{
				throw 1;
			}

			if (padSize)
			{
				fseek(fp, padSize, SEEK_CUR);
			}
		}

		delete[] bmpRow;
	}
	catch (int32_t err)
	{
		switch (err)
		{
			case 0:
				Log::E("File %s can't be opened for writing", relativePath);
				break;
			case 1:
				Log::E("Can't write to file.");
				break;
			case 2:
				Log::E("Only PF_LA_8, PF_RGB_8 and PF_RGBA_8 image formats are supported.");
				break;
		}
	}

	if (fp)
		fclose(fp);
}

} // namespace crown

