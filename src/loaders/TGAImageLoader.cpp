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
#include "Str.h"
#include "TGAImageLoader.h"
#include "Filesystem.h"

namespace Crown
{

TGAImageLoader::TGAImageLoader()
{
}

TGAImageLoader::~TGAImageLoader()
{
}

Image* TGAImageLoader::LoadFile(const char* relativePath)
{
	Stream* fileStream;

	fileStream = GetFilesystem()->OpenStream(relativePath, SOM_READ);

	if (fileStream == NULL)
	{
		return NULL;
	}

	fileStream->ReadDataBlock(&mTGAHeader, sizeof(mTGAHeader));
	// Skip ID
	fileStream->Seek(mTGAHeader.id_length, SM_SeekFromCurrent);

	Image* image = NULL;

	switch (mTGAHeader.image_type)
	{
		case 0:
			Log::E("The file does not contain image data.");
			return NULL;
			break;
		case 2:
			image = LoadUncompressedData(fileStream);
			break;
		case 10:
			image = LoadCompressedData(fileStream);
			break;
		default:
			Log::E("Data type not supported.");
			return NULL;
			break;
	}

	GetFilesystem()->Close(fileStream);

	return image;
}

Image* TGAImageLoader::LoadUncompressedData(Stream* fp)
{
	Image* image;
	uint channels = mTGAHeader.pixel_depth / 8;
	ulong size = mTGAHeader.width * mTGAHeader.height;
	uchar* data;
	PixelFormat format = PF_RGB_8;

	if (mTGAHeader.pixel_depth == 16)
	{
		data = new uchar[(uint)(size * 3)];
		int j = 0;

		for (ulong i = 0; i < size * channels; i++)
		{
			ushort pixel_data;
			fp->ReadDataBlock(&pixel_data, sizeof(pixel_data));
			data[j] = (pixel_data & 0x7c) >> 10;
			data[j+1] = (pixel_data & 0x3e) >> 5;
			data[j+2] = (pixel_data & 0x1f);
			j += 3;
		}
	}
	else
	{
		data = new uchar[(uint)(size * channels)];
		fp->ReadDataBlock(data, (size_t)(size * channels));
		SwapRedBlue(data, size * channels, channels);
	}

	if (channels == 4)
	{
		format = PF_RGBA_8;
	}

	image = new Image(format, mTGAHeader.width, mTGAHeader.height, data);
	return image;
}

Image* TGAImageLoader::LoadCompressedData(Stream* fp)
{
	Image* image;
	uint channels = mTGAHeader.pixel_depth/8;
	uchar rle_id = 0;
	uint i = 0;
	uchar* colors;
	uint colors_read = 0;
	ulong size = mTGAHeader.width * mTGAHeader.height;
	uchar* data = new uchar[(uint)(size * channels)];
	colors = new uchar[channels];
	PixelFormat format = PF_RGB_8;

	if (channels == 4)
	{
		format = PF_RGBA_8;
	}

	while (i < size)
	{
		fp->ReadDataBlock(&rle_id, sizeof(uchar));

		if (rle_id & 0x80)   // Se il bit più significativo è ad 1
		{
			rle_id -= 127;
			fp->ReadDataBlock(colors, channels);

			while (rle_id)
			{
				data[colors_read] = colors[2];
				data[colors_read+1] = colors[1];
				data[colors_read+2] = colors[0];

				if (channels == 4)
				{
					data[colors_read+3] = colors[3];
				}

				rle_id--;
				colors_read += channels;
				i++;
			}
		}
		else     // Altrimenti leggi i colori normalmente
		{
			rle_id++;

			while (rle_id)
			{
				fp->ReadDataBlock(colors, channels);
				data[colors_read] = colors[2];
				data[colors_read+1] = colors[1];
				data[colors_read+2] = colors[0];

				if (channels == 4)
				{
					data[colors_read+3] = colors[3];
				}

				rle_id--;
				colors_read += channels;
				i++;
			}
		}
	}

	delete[] colors;
	image = new Image(format, mTGAHeader.width, mTGAHeader.height, data);
	return image;
}

void TGAImageLoader::SwapRedBlue(uchar* data, ulong size, uint channels)
{
	for (ulong i = 0; i < size; i += channels)
	{
		data[i] ^= data[i+2];
		data[i+2] ^= data[i];
		data[i] ^= data[i+2];
	}
}

void TGAImageLoader::SaveFile(const Image* image, const char* relativePath)
{
	if (image->GetFormat() != PF_RGB_8 || image->GetFormat() != PF_RGBA_8)
	{
		Log::E("TGAImageLoader::SaveFile: Only PF_RGB8 and PF_RGBA8 supported");
		return;
	}

	TGAHeader_t header;

	header.id_length			= 0;
	header.color_map_type		= 0;
	header.image_type			= 2; // Uncompressed RGB
	header.c_map_spec[0]		= 0;
	header.c_map_spec[1]		= 0;
	header.c_map_spec[2]		= 0;
	header.c_map_spec[3]		= 0;
	header.c_map_spec[4]		= 0;
	header.x_offset				= 0;
	header.y_offset				= 0;
	header.width				= image->GetWidth();
	header.height				= image->GetHeight();
	header.pixel_depth			= image->GetBitsPerPixel();
	header.image_descriptor		= 0;

	Stream* fileStream = GetFilesystem()->OpenStream(relativePath, SOM_WRITE);

	if (fileStream)
	{
		fileStream->WriteDataBlock(&header, sizeof(header));
		fileStream->WriteDataBlock(image->GetBuffer(), image->GetWidth() * image->GetHeight() * image->GetBytesPerPixel());

		GetFilesystem()->Close(fileStream);
	}
}

} // namespace Crown

