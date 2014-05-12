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

#include "Allocator.h"
#include "Filesystem.h"
#include "PixelFormat.h"
#include "TextureResource.h"
#include "ReaderWriter.h"
#include "JSONParser.h"

namespace crown
{
namespace texture_resource
{

struct ImageData
{
	uint32_t width;
	uint32_t height;
	PixelFormat::Enum format;
	uint32_t num_mips;
	char* data;
};

//-----------------------------------------------------------------------------
void swap_red_blue(uint32_t width, uint32_t height, uint8_t channels, char* data)
{
	uint32_t i = 0;

	for (uint32_t h = 0; h < height; h++)
	{
		for (uint32_t w = 0; w < width; w++)
		{
			const uint8_t tmp = data[i + 0];
			data[i + 0] = data[i + 2];
			data[i + 2] = tmp;

			i += channels;
		}
	}
}

//-----------------------------------------------------------------------------
void read_tga_uncompressed(BinaryReader& br, uint32_t width, uint32_t height, uint8_t channels, ImageData& image)
{
	if (channels == 2)
	{
		uint32_t i = 0;

		for (uint32_t h = 0; h < height; h++)
		{
			for (uint32_t w = 0; w < width; w++)
			{
				uint16_t data;
				br.read(data);

				image.data[i + 0] = (data & 0x7c) >> 10;
				image.data[i + 1] = (data & 0x3e) >> 5;
				image.data[i + 2] = (data & 0x1f);

				i += 3;
			}
		}
	}
	else
	{
		br.read(image.data, width * height * channels);
		swap_red_blue(width, height, channels, image.data);
	}
}

//-----------------------------------------------------------------------------
void read_tga_compressed(BinaryReader& br, uint32_t width, uint32_t height, uint8_t channels, ImageData& image)
{
	uint8_t rle_id = 0;
	uint32_t i = 0;
	uint32_t colors_read = 0;

	// Can't be more than 4 channels
	uint8_t colors[4];

	while (i < width * height)
	{
		br.read(rle_id);

		// If MSB == 1
		if (rle_id & 0x80)
		{
			rle_id -= 127;
			
			br.read(colors[0]);
			br.read(colors[1]);
			br.read(colors[2]);

			if (channels == 4)
				br.read(colors[3]);

			while (rle_id)
			{
				image.data[colors_read + 0] = colors[2];
				image.data[colors_read + 1] = colors[1];
				image.data[colors_read + 2] = colors[0];

				if (channels == 4)
					image.data[colors_read + 3] = colors[3];

				rle_id--;
				colors_read += channels;
				i++;
			}
		}
		else
		{
			rle_id++;

			while (rle_id)
			{
				br.read(colors[0]);
				br.read(colors[1]);
				br.read(colors[2]);

				if (channels == 4)
					br.read(colors[3]);
				
				image.data[colors_read + 0] = colors[2];
				image.data[colors_read + 1] = colors[1];
				image.data[colors_read + 2] = colors[0];

				if (channels == 4)
					image.data[colors_read + 3] = colors[3];

				rle_id--;
				colors_read += channels;
				i++;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void parse_tga(BinaryReader& br, ImageData& image)
{
	uint8_t id;
	br.read(id);

	uint8_t cmap_type;
	br.read(cmap_type);

	uint8_t image_type;
	br.read(image_type);

	uint8_t garbage;
	for (uint32_t i = 0; i < 5; i++)
		br.read(garbage);

	uint16_t x_offt;
	br.read(x_offt);

	uint16_t y_offt;
	br.read(y_offt);

	uint16_t width;
	br.read(width);

	uint16_t height;
	br.read(height);

	uint8_t depth;
	br.read(depth);

	uint8_t desc;
	br.read(desc);

	// Skip TGA ID
	br.skip(id);

	CE_ASSERT(image_type != 0, "TGA does not contain image data");
	CE_ASSERT(image_type == 2 || image_type == 10, "TGA image format not supported");

	const uint32_t channels = depth / 8;

	image.width = width;
	image.height = height;
	image.num_mips = 1;

	switch (channels)
	{
		case 2: image.format = PixelFormat::R8G8B8; break;
		case 3: image.format = PixelFormat::R8G8B8; break;
		case 4: image.format = PixelFormat::R8G8B8A8; break;
		default: CE_FATAL("TGA channels not supported"); break;
	}

	image.data = (char*) default_allocator().allocate(Pixel::bytes_per_pixel(image.format) * width * height);

	if (image_type == 2)
	{
		read_tga_uncompressed(br, width, height, channels, image);
	}
	else if (image_type == 10)
	{
		read_tga_compressed(br, width, height, channels, image);
	}

	return;
}

static DdsPixelFormat PIXEL_FORMAT_TO_DDSPF[PixelFormat::COUNT] =
{
	{ DDPF_HEADERSIZE, DDS_FOURCC, DDFP_FOURCC_DXT1, 0, 0, 0, 0, 0 }, // DXT1
	{ DDPF_HEADERSIZE, DDS_FOURCC, DDFP_FOURCC_DXT3, 0, 0, 0, 0, 0 }, // DXT3
	{ DDPF_HEADERSIZE, DDS_FOURCC, DDFP_FOURCC_DXT5, 0, 0, 0, 0, 0 }, // DXT5

	{ DDPF_HEADERSIZE, DDS_RGB,    0,                24, 0xFF0000, 0xFF00, 0xFF, 0}, // R8G8B8
	{ DDPF_HEADERSIZE, DDS_RGBA,   0,                32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF}, // R8G8B8A8

	{ 0, 0, 0, 0, 0, 0, 0, 0}, // D16
	{ 0, 0, 0, 0, 0, 0, 0, 0}, // D24
	{ 0, 0, 0, 0, 0, 0, 0, 0}, // D32
	{ 0, 0, 0, 0, 0, 0, 0, 0}, // D24S8
};

//-----------------------------------------------------------------------------
void parse_dds(BinaryReader& br, ImageData& image)
{
	// Read header
	uint32_t magic;
	br.read(magic);
	CE_ASSERT(magic == DDSD_MAGIC, "DDS bad magic number");

	uint32_t hsize;
	br.read(hsize);
	CE_ASSERT(hsize == DDSD_HEADERSIZE, "DDS bas header size");

	uint32_t flags;
	br.read(flags);
	CE_ASSERT(flags & (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT), "DDS bad header flags");

	uint32_t height;
	br.read(height);

	uint32_t width;
	br.read(width);

	uint32_t pitch;
	br.read(pitch);

	uint32_t depth;
	br.read(depth);

	uint32_t num_mips;
	br.read(num_mips);

	// Skip reserved bits
	br.skip(sizeof(uint32_t) * 11);

	// Read pixel format
	uint32_t pf_hsize;
	br.read(pf_hsize);
	CE_ASSERT(pf_hsize == DDPF_HEADERSIZE, "DDS bad pf header size");

	uint32_t pf_flags;
	br.read(pf_flags);

	uint32_t pf_fourcc;
	br.read(pf_fourcc);

	uint32_t pf_bitcount;
	br.read(pf_bitcount);

	uint32_t pf_rmask;
	br.read(pf_rmask);

	uint32_t pf_gmask;
	br.read(pf_gmask);

	uint32_t pf_bmask;
	br.read(pf_bmask);

	uint32_t pf_amask;
	br.read(pf_amask);

	uint32_t caps;
	br.read(caps);
	CE_ASSERT((caps & DDSCAPS_TEXTURE), "DDS bad caps");

	uint32_t caps2;
	br.read(caps2);

	uint32_t caps3;
	br.read(caps3);

	uint32_t caps4;
	br.read(caps4);

	uint32_t reserved2;
	br.read(reserved2);

	image.width = width;
	image.height = height;
	image.num_mips = (flags & DDSD_MIPMAPCOUNT) ? num_mips : 1;
	image.format = PixelFormat::COUNT;

	DdsPixelFormat ddspf;
	ddspf.size = pf_hsize;
	ddspf.flags = pf_flags;
	ddspf.fourcc = pf_fourcc;
	ddspf.bitcount = pf_bitcount;
	ddspf.rmask = pf_rmask;
	ddspf.gmask = pf_gmask;
	ddspf.bmask = pf_bmask;
	ddspf.amask = pf_amask;

	for (uint32_t i = 0; i < PixelFormat::COUNT; i++)
	{
		if (memcmp(&ddspf, &PIXEL_FORMAT_TO_DDSPF[(PixelFormat::Enum) i], sizeof(DdsPixelFormat)) == 0)
		{
			image.format = (PixelFormat::Enum) i;
		}
	}

	CE_ASSERT(image.format != PixelFormat::COUNT, "DDS pixel format not supported");
}

//-----------------------------------------------------------------------------
void write_dds(BinaryWriter& bw, const ImageData& image)
{
	bw.write(DDSD_MAGIC);

	// Header
	bw.write(DDSD_HEADERSIZE); // dwSize
	bw.write(DDS_HEADER_FLAGS_TEXTURE | DDSD_PITCH | DDSD_MIPMAPCOUNT); // dwFlags
	bw.write(image.height); // dwHeight
	bw.write(image.width); // dwWidth
	const uint32_t pitch = uint32_t(( image.width * 32 + 7 ) / 8);
	bw.write(pitch); // dwPitchOrLinearSize
	bw.write(DDSD_UNUSED); // dwDepth
	bw.write(image.num_mips); // dwMipMapCount;

	for (uint32_t i = 0; i < 11; i++)
		bw.write(DDSD_UNUSED); // dwReserved1[11];

	// PixelFormat
	bw.write(DDPF_HEADERSIZE); // dwSize;
	const uint32_t pf = DDPF_RGB | (image.format == PixelFormat::R8G8B8A8 ? DDPF_ALPHAPIXELS : 0);
	bw.write(pf); // dwFlags;
	bw.write(DDSD_UNUSED); // dwFourCC;
	bw.write(uint32_t(image.format == PixelFormat::R8G8B8A8 ? 32 : 24)); // dwRGBBitCount;
	bw.write(uint32_t(0x000000FF)); // dwRBitMask;
	bw.write(uint32_t(0x0000FF00)); // dwGBitMask;
	bw.write(uint32_t(0x00FF0000)); // dwBBitMask;
	bw.write(uint32_t(0xFF000000)); // dwABitMask;

	bw.write(DDSCAPS_TEXTURE); // dwCaps;
	bw.write(DDSD_UNUSED); // dwCaps2;
	bw.write(DDSD_UNUSED); // dwCaps3;
	bw.write(DDSD_UNUSED); // dwCaps4;
	bw.write(DDSD_UNUSED); // dwReserved2;

	const char* data = image.data;
	for (uint32_t i = 0; i < image.height; i++)
	{
		bw.write((const void*) data, pitch);
		data += pitch;
	}
}

//-----------------------------------------------------------------------------
void compile(Filesystem& fs, const char* resource_path, File* out_file)
{
	File* in_file = fs.open(resource_path, FOM_READ);
	JSONParser json(*in_file);
	fs.close(in_file);

	// Read source file
	JSONElement root = json.root();
	DynamicString name;
	root.key("source").to_string(name);

	File* source = fs.open(name.c_str(), FOM_READ);
	BinaryReader br(*source);
	ImageData image;

	if (name.ends_with(".tga"))
	{
		parse_tga(br, image);
	}
	else if (name.ends_with(".dds"))
	{
		// parse_dds(br, image);
		CE_FATAL(".dds not supported");
	}
	else
	{
		CE_FATAL("Source image not supported");
	}

	fs.close(source);

	BinaryWriter bw(*out_file);
	
	// Write texture header
	bw.write(uint32_t(0x00000000));

	// Write DDS
	write_dds(bw, image);

	default_allocator().deallocate(image.data);
}

} // namespace texture_resource
} // namespace crown
