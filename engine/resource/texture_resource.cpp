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

#include "allocator.h"
#include "filesystem.h"
#include "texture_resource.h"
#include "reader_writer.h"
#include "json_parser.h"
#include "math_utils.h"
#include "log.h"

namespace crown
{

#define FOURCC(a, b, c, d)			uint32_t(a | (b << 8) | (c << 16) | (d << 24))

#define DDSD_MAGIC 					FOURCC('D', 'D', 'S', ' ')
#define DDSD_HEADERSIZE				uint32_t(124)
#define DDSD_UNUSED					uint32_t(0x00000000)
#define DDSD_CAPS					uint32_t(0x00000001) // Required in every .dds file.
#define DDSD_HEIGHT					uint32_t(0x00000002) // Required in every .dds file.
#define DDSD_WIDTH					uint32_t(0x00000004) // Required in every .dds file.
#define DDSD_PITCH					uint32_t(0x00000008) // Required when pitch is provided for an uncompressed texture.
#define DDSD_PIXELFORMAT			uint32_t(0x00001000) // Required in every .dds file.
#define DDSD_MIPMAPCOUNT			uint32_t(0x00020000) // Required in a mipmapped texture.
#define DDSD_LINEARSIZE				uint32_t(0x00080000) // Required when pitch is provided for a compressed texture.
#define DDSD_DEPTH					uint32_t(0x00800000) // Required in a depth texture.

#define DDS_HEADER_FLAGS_TEXTURE	uint32_t(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT)
#define DDS_HEADER_FLAGS_MIPMAP		uint32_t(DDSD_MIPMAPCOUNT)
#define DDS_HEADER_FLAGS_VOLUME		uint32_t(DDSD_DEPTH)
#define DDS_HEADER_FLAGS_PITCH		uint32_t(DDSD_PITCH)
#define DDS_HEADER_FLAGS_LINEARSIZE	uint32_t(DDSD_LINEARSIZE)

#define DDSCAPS_COMPLEX				uint32_t(0x00000008) // Optional; must be used on any file that contains more than one surface (a mipmap, a cubic environment map, or mipmapped volume texture).
#define DDSCAPS_MIPMAP				uint32_t(0x00400000) // Optional; should be used for a mipmap.
#define DDSCAPS_TEXTURE				uint32_t(0x00001000) // Required

#define DDSCAPS2_CUBEMAP			uint32_t(0x00000200) // Required for a cube map.
#define DDSCAPS2_CUBEMAP_POSITIVEX	uint32_t(0x00000400) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_NEGATIVEX	uint32_t(0x00000800) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_POSITIVEY	uint32_t(0x00001000) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_NEGATIVEY	uint32_t(0x00002000) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_POSITIVEZ	uint32_t(0x00004000) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_NEGATIVEZ	uint32_t(0x00008000) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_VOLUME				uint32_t(0x00200000) // Required for a volume texture.

#define DDPF_HEADERSIZE				uint32_t(32)
#define DDPF_ALPHAPIXELS			uint32_t(0x00000001) // Texture contains alpha data; dwRGBAlphaBitMask contains valid data.
#define DDPF_ALPHA					uint32_t(0x00000002) // Used in some older DDS files for alpha channel only uncompressed data (dwRGBBitCount contains the alpha channel bitcount; dwABitMask contains valid data)
#define DDPF_FOURCC					uint32_t(0x00000004) // Texture contains compressed RGB data; dwFourCC contains valid data.
#define DDPF_RGB					uint32_t(0x00000040) // Texture contains uncompressed RGB data; dwRGBBitCount and the RGB masks (dwRBitMask, dwGBitMask, dwBBitMask) contain valid data.
#define DDPF_YUV					uint32_t(0x00000200) // Used in some older DDS files for YUV uncompressed data (dwRGBBitCount contains the YUV bit count; dwRBitMask contains the Y mask, dwGBitMask contains the U mask, dwBBitMask contains the V mask)
#define DDPF_LUMINANCE				uint32_t(0x00020000) // Used in some older DDS files for single channel color uncompressed data (dwRGBBitCount contains the luminance channel bit count; dwRBitMask contains the channel mask). Can be combined with DDPF_ALPHAPIXELS for a two channel DDS file.

#define DDS_FOURCC					uint32_t(DDPF_FOURCC)
#define DDS_RGB						uint32_t(DDPF_RGB)
#define DDS_RGBA					uint32_t(DDPF_RGB | DDPF_ALPHAPIXELS)
#define DDS_LUMINANCE				uint32_t(DDPF_LUMINANCE)
#define DDS_LUMINANCEA				uint32_t(DDPF_LUMINANCE | DDPF_ALPHAPIXELS)
#define DDS_ALPHA					uint32_t(DDPF_ALPHA)

#define DDPF_FOURCC_DXT1			FOURCC('D', 'X', 'T', '1')
#define DDPF_FOURCC_DXT2			FOURCC('D', 'X', 'T', '2')
#define DDPF_FOURCC_DXT3			FOURCC('D', 'X', 'T', '3')
#define DDPF_FOURCC_DXT4			FOURCC('D', 'X', 'T', '4')
#define DDPF_FOURCC_DXT5			FOURCC('D', 'X', 'T', '5')
#define DDPF_FOURCC_DX10			FOURCC('D', 'X', '1', '0')

#define DDS_HEADER_OFFSET			uint32_t(sizeof(TextureHeader))
#define DDS_DATA_OFFSET				uint32_t(DDS_HEADER_OFFSET + DDSD_HEADERSIZE)

struct DdsPixelFormat
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourcc;
	uint32_t bitcount;
	uint32_t rmask;
	uint32_t gmask;
	uint32_t bmask;
	uint32_t amask;
};

struct DdsHeader
{
	uint32_t		magic;
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitch_or_linear_size;
	uint32_t        depth;
	uint32_t        num_mips;
	uint32_t        reserved[11];
	DdsPixelFormat  ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

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

namespace pixel_format
{
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

	inline bool is_compressed(PixelFormat::Enum fmt)
	{
		return fmt < PixelFormat::R8G8B8;
	}

	inline bool is_color(PixelFormat::Enum fmt)
	{
		return fmt >= PixelFormat::R8G8B8 && fmt < PixelFormat::D16;
	}

	inline bool is_depth(PixelFormat::Enum fmt)
	{
		return fmt >= PixelFormat::D16 && fmt < PixelFormat::COUNT;
	}
} // namespace pixel_format

namespace texture_resource
{
	struct ImageData
	{
		uint32_t width;
		uint32_t height;
		uint32_t pitch;
		PixelFormat::Enum format;
		uint32_t num_mips;
		char* data;
	};

	struct MipData
	{
		uint32_t width;
		uint32_t height;
		PixelFormat::Enum format;
		uint32_t size;
		char* data;
	};

	void read_mip_image(const ImageData& image, uint8_t mip, MipData& data)
	{
		uint32_t width = image.width;
		uint32_t height = image.height;
		//uint32_t pitch = image.pitch;
		uint32_t cur_mip = 0;
		char* src = image.data;

		while (1)
		{
			const uint32_t size = width * height * pixel_format::size(image.format);

			if (cur_mip == mip)
			{
				data.width = width;
				data.height = height;
				data.format = image.format;
				data.size = size;
				data.data = src;
				return;
			}

			width = math::max(1u, width >> 1);
			height = math::max(1u, height >> 1);
			cur_mip++;
			src += size;
		}
	}

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

		swap_red_blue(width, height, channels, image.data);
	}

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

		image.data = (char*) default_allocator().allocate(pixel_format::size(image.format) * width * height);

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

		CE_LOGD("width = %u", width);
		CE_LOGD("height = %u", height);
		CE_LOGD("mips = %u", num_mips);
		CE_LOGD("pitch = %u (valid = %s)", pitch, flags & DDSD_PITCH ? "yes" : "no");
		CE_LOGD("pfflags = %.8x", pf_flags);

		image.width = width;
		image.height = height;
		image.pitch = pitch;
		image.num_mips = (flags & DDSD_MIPMAPCOUNT) ? num_mips : 1;
		image.data = (char*) (uintptr_t) DDS_DATA_OFFSET;

		const uint32_t raw_fmt = (pf_flags & DDPF_FOURCC) ? pf_fourcc : pf_flags;
		switch (raw_fmt)
		{
			case DDPF_FOURCC_DXT1: image.format = PixelFormat::DXT1; break;
			case DDPF_FOURCC_DXT3: image.format = PixelFormat::DXT3; break;
			case DDPF_FOURCC_DXT5: image.format = PixelFormat::DXT5; break;
			case DDS_RGB: image.format = PixelFormat::R8G8B8; break;
			case DDS_RGBA: image.format = PixelFormat::R8G8B8A8; break;
			default: image.format = PixelFormat::COUNT; break;
		}

		CE_ASSERT(image.format != PixelFormat::COUNT, "DDS pixel format not supported");
		CE_LOGD("PixelFormat = %u", image.format);
	}

	void write_dds(BinaryWriter& bw, const ImageData& image)
	{
		bw.write(DDSD_MAGIC);

		// Header
		bw.write(DDSD_HEADERSIZE); // dwSize
		bw.write(DDS_HEADER_FLAGS_TEXTURE
			| DDSD_MIPMAPCOUNT
			| (pixel_format::is_compressed(image.format) ? DDSD_LINEARSIZE : DDSD_PITCH)
			| (image.num_mips ? DDSD_MIPMAPCOUNT : 0)); // dwFlags
		bw.write(image.height); // dwHeight
		bw.write(image.width); // dwWidth

		const uint32_t pitch = pixel_format::is_compressed(image.format) ? 0 // fixme
								: (image.width * pixel_format::size(image.format) * 8 + 7) / 8;

		bw.write(pitch); // dwPitchOrLinearSize
		bw.write(DDSD_UNUSED); // dwDepth
		bw.write(image.num_mips); // dwMipMapCount;

		for (uint32_t i = 0; i < 11; i++)
			bw.write(DDSD_UNUSED); // dwReserved1[11];

		// Pixel format
		bw.write(DDPF_HEADERSIZE); // dwSize;
		uint32_t pf = 0;
		switch (image.format)
		{
			case PixelFormat::DXT1:     pf = DDPF_FOURCC_DXT1; break;
			case PixelFormat::DXT3:     pf = DDPF_FOURCC_DXT3; break;
			case PixelFormat::DXT5:     pf = DDPF_FOURCC_DXT5; break;
			case PixelFormat::R8G8B8:   pf = DDS_RGB; break;
			case PixelFormat::R8G8B8A8: pf = DDS_RGBA; break;
			default: CE_FATAL("Pixel format unknown"); break;
		}
		bw.write(pixel_format::is_compressed(image.format) ? DDPF_FOURCC : pf); // dwFlags;
		bw.write(pixel_format::is_compressed(image.format) ? pf : DDSD_UNUSED); // dwFourCC;
		bw.write(uint32_t(pixel_format::size(image.format) * 8)); // dwRGBBitCount;
		bw.write(uint32_t(0x00FF0000)); // dwRBitMask;
		bw.write(uint32_t(0x0000FF00)); // dwGBitMask;
		bw.write(uint32_t(0x000000FF)); // dwBBitMask;
		bw.write(uint32_t(0xFF000000)); // dwABitMask;

		bw.write(DDSCAPS_TEXTURE
			| (image.num_mips > 1 ? DDSCAPS_COMPLEX : DDSD_UNUSED) // also for cubemap, depth mipmap
			| (image.num_mips > 1 ? DDSCAPS_MIPMAP : DDSD_UNUSED)); // dwCaps;
		bw.write(DDSD_UNUSED); // dwCaps2;
		bw.write(DDSD_UNUSED); // dwCaps3;
		bw.write(DDSD_UNUSED); // dwCaps4;
		bw.write(DDSD_UNUSED); // dwReserved2;

		// Image data
		for (uint32_t i = 0; i < image.num_mips; i++)
		{
			MipData mip;
			read_mip_image(image, i, mip);

			// CE_LOGD("Writing mip: (%ux%u) byes = %u", mip.width, mip.height, mip.size);
			bw.write(mip.data, mip.size);
		}
	}

	void compile(const char* path, CompileOptions& opts)
	{
		static const uint32_t VERSION = 1;

		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
		JSONElement root = json.root();

		DynamicString name;
		root.key("source").to_string(name);

		File* source = opts._fs.open(name.c_str(), FOM_READ);
		BinaryReader br(*source);
		ImageData image;

		if (name.ends_with(".tga"))
		{
			parse_tga(br, image);
		}
		else if (name.ends_with(".dds"))
		{
			// parse_dds(br, image);
			// size_t size = source->size();
			// image.data = (char*) default_allocator().allocate(size);
			// source->seek(0);
			// source->read(image.data, size);
			// image.data += DDS_DATA_OFFSET;

			// BinaryWriter bw(*out_file);
			// write_dds(bw, image);
		}
		else
		{
			CE_FATAL("Source image not supported");
		}

		opts._fs.close(source);

		// Write DDS
		opts.write(VERSION); // Version
		opts.write(uint32_t(0)); // Size
		write_dds(opts._bw, image);

		default_allocator().deallocate(image.data);
	}

	void* load(File& file, Allocator& a)
	{
		const size_t file_size = file.size();
		file.skip(sizeof(TextureHeader));
		const bgfx::Memory* mem = bgfx::alloc(file_size);
		file.read(mem->data, file_size - sizeof(TextureHeader));

		TextureResource* teximg = (TextureResource*) a.allocate(sizeof(TextureResource));
		teximg->mem = mem;
		teximg->handle.idx = bgfx::invalidHandle;

		return teximg;
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		TextureResource* teximg = (TextureResource*) rm.get(TEXTURE_TYPE, id);
		teximg->handle = bgfx::createTexture(teximg->mem);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		TextureResource* teximg = (TextureResource*) rm.get(TEXTURE_TYPE, id);
		bgfx::destroyTexture(teximg->handle);
	}
	
	void unload(Allocator& a, void* resource)
	{
		a.deallocate(resource);
	}

} // namespace texture_resource
} // namespace crown
