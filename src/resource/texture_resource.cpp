/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "filesystem.h"
#include "texture_resource.h"
#include "reader_writer.h"
#include "math_utils.h"
#include "resource_manager.h"
#include "log.h"
#include "compile_options.h"
#include "sjson.h"
#include "map.h"
#include <algorithm>

namespace crown
{
#define FOURCC(a, b, c, d)          u32(a | (b << 8) | (c << 16) | (d << 24))

#define DDSD_MAGIC                  FOURCC('D', 'D', 'S', ' ')
#define DDSD_HEADERSIZE             u32(124)
#define DDSD_UNUSED                 u32(0x00000000)
#define DDSD_CAPS                   u32(0x00000001) // Required in every .dds file.
#define DDSD_HEIGHT                 u32(0x00000002) // Required in every .dds file.
#define DDSD_WIDTH                  u32(0x00000004) // Required in every .dds file.
#define DDSD_PITCH                  u32(0x00000008) // Required when pitch is provided for an uncompressed texture.
#define DDSD_PIXELFORMAT            u32(0x00001000) // Required in every .dds file.
#define DDSD_MIPMAPCOUNT            u32(0x00020000) // Required in a mipmapped texture.
#define DDSD_LINEARSIZE             u32(0x00080000) // Required when pitch is provided for a compressed texture.
#define DDSD_DEPTH                  u32(0x00800000) // Required in a depth texture.

#define DDS_HEADER_FLAGS_TEXTURE    u32(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT)
#define DDS_HEADER_FLAGS_MIPMAP     u32(DDSD_MIPMAPCOUNT)
#define DDS_HEADER_FLAGS_VOLUME     u32(DDSD_DEPTH)
#define DDS_HEADER_FLAGS_PITCH      u32(DDSD_PITCH)
#define DDS_HEADER_FLAGS_LINEARSIZE u32(DDSD_LINEARSIZE)

#define DDSCAPS_COMPLEX             u32(0x00000008) // Optional; must be used on any file that contains more than one surface (a mipmap, a cubic environment map, or mipmapped volume texture).
#define DDSCAPS_MIPMAP              u32(0x00400000) // Optional; should be used for a mipmap.
#define DDSCAPS_TEXTURE             u32(0x00001000) // Required

#define DDSCAPS2_CUBEMAP            u32(0x00000200) // Required for a cube map.
#define DDSCAPS2_CUBEMAP_POSITIVEX  u32(0x00000400) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_NEGATIVEX  u32(0x00000800) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_POSITIVEY  u32(0x00001000) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_NEGATIVEY  u32(0x00002000) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_POSITIVEZ  u32(0x00004000) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  u32(0x00008000) // Required when these surfaces are stored in a cube map.
#define DDSCAPS2_VOLUME             u32(0x00200000) // Required for a volume texture.

#define DDPF_HEADERSIZE             u32(32)
#define DDPF_ALPHAPIXELS            u32(0x00000001) // Texture contains alpha data; dwRGBAlphaBitMask contains valid data.
#define DDPF_ALPHA                  u32(0x00000002) // Used in some older DDS files for alpha channel only uncompressed data (dwRGBBitCount contains the alpha channel bitcount; dwABitMask contains valid data)
#define DDPF_FOURCC                 u32(0x00000004) // Texture contains compressed RGB data; dwFourCC contains valid data.
#define DDPF_RGB                    u32(0x00000040) // Texture contains uncompressed RGB data; dwRGBBitCount and the RGB masks (dwRBitMask, dwGBitMask, dwBBitMask) contain valid data.
#define DDPF_YUV                    u32(0x00000200) // Used in some older DDS files for YUV uncompressed data (dwRGBBitCount contains the YUV bit count; dwRBitMask contains the Y mask, dwGBitMask contains the U mask, dwBBitMask contains the V mask)
#define DDPF_LUMINANCE              u32(0x00020000) // Used in some older DDS files for single channel color uncompressed data (dwRGBBitCount contains the luminance channel bit count; dwRBitMask contains the channel mask). Can be combined with DDPF_ALPHAPIXELS for a two channel DDS file.

#define DDS_FOURCC                  u32(DDPF_FOURCC)
#define DDS_RGB                     u32(DDPF_RGB)
#define DDS_RGBA                    u32(DDPF_RGB | DDPF_ALPHAPIXELS)
#define DDS_LUMINANCE               u32(DDPF_LUMINANCE)
#define DDS_LUMINANCEA              u32(DDPF_LUMINANCE | DDPF_ALPHAPIXELS)
#define DDS_ALPHA                   u32(DDPF_ALPHA)

#define DDPF_FOURCC_DXT1            FOURCC('D', 'X', 'T', '1')
#define DDPF_FOURCC_DXT2            FOURCC('D', 'X', 'T', '2')
#define DDPF_FOURCC_DXT3            FOURCC('D', 'X', 'T', '3')
#define DDPF_FOURCC_DXT4            FOURCC('D', 'X', 'T', '4')
#define DDPF_FOURCC_DXT5            FOURCC('D', 'X', 'T', '5')
#define DDPF_FOURCC_DX10            FOURCC('D', 'X', '1', '0')

#define DDS_HEADER_OFFSET           u32(sizeof(TextureHeader))
#define DDS_DATA_OFFSET             u32(DDS_HEADER_OFFSET + DDSD_HEADERSIZE)

struct DdsPixelFormat
{
	u32 size;
	u32 flags;
	u32 fourcc;
	u32 bitcount;
	u32 rmask;
	u32 gmask;
	u32 bmask;
	u32 amask;
};

struct DdsHeader
{
	u32 magic;
	u32 size;
	u32 flags;
	u32 height;
	u32 width;
	u32 pitch_or_linear_size;
	u32 depth;
	u32 num_mips;
	u32 reserved[11];
	DdsPixelFormat ddspf;
	u32 caps;
	u32 caps2;
	u32 caps3;
	u32 caps4;
	u32 reserved2;
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
	inline u32 size(PixelFormat::Enum fmt)
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
		u32 width;
		u32 height;
		u32 pitch;
		PixelFormat::Enum format;
		u32 num_mips;
		char* data;
	};

	struct MipData
	{
		u32 width;
		u32 height;
		PixelFormat::Enum format;
		u32 size;
		char* data;
	};

	void read_mip_image(const ImageData& image, u8 mip, MipData& data)
	{
		u32 width = image.width;
		u32 height = image.height;
		//u32 pitch = image.pitch;
		u32 cur_mip = 0;
		char* src = image.data;

		while (1)
		{
			const u32 size = width * height * pixel_format::size(image.format);

			if (cur_mip == mip)
			{
				data.width = width;
				data.height = height;
				data.format = image.format;
				data.size = size;
				data.data = src;
				return;
			}

			width = std::max(1u, width >> 1);
			height = std::max(1u, height >> 1);
			cur_mip++;
			src += size;
		}
	}

	void swap_red_blue(u32 width, u32 height, u8 channels, char* data)
	{
		u32 i = 0;

		for (u32 h = 0; h < height; h++)
		{
			for (u32 w = 0; w < width; w++)
			{
				const u8 tmp = data[i + 0];
				data[i + 0] = data[i + 2];
				data[i + 2] = tmp;

				i += channels;
			}
		}
	}

	void read_tga_uncompressed(BinaryReader& br, u32 width, u32 height, u8 channels, ImageData& image)
	{
		if (channels == 2)
		{
			u32 i = 0;

			for (u32 h = 0; h < height; h++)
			{
				for (u32 w = 0; w < width; w++)
				{
					u16 data;
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

	void read_tga_compressed(BinaryReader& br, u32 width, u32 height, u8 channels, ImageData& image)
	{
		u8 rle_id = 0;
		u32 i = 0;
		u32 colors_read = 0;

		// Can't be more than 4 channels
		u8 colors[4];

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
		u8 id;
		br.read(id);

		u8 cmap_type;
		br.read(cmap_type);

		u8 image_type;
		br.read(image_type);

		u8 garbage;
		for (u32 i = 0; i < 5; i++)
			br.read(garbage);

		u16 x_offt;
		br.read(x_offt);

		u16 y_offt;
		br.read(y_offt);

		u16 width;
		br.read(width);

		u16 height;
		br.read(height);

		u8 depth;
		br.read(depth);

		u8 desc;
		br.read(desc);

		// Skip TGA ID
		br.skip(id);

		CE_ASSERT(image_type != 0, "TGA does not contain image data");
		CE_ASSERT(image_type == 2 || image_type == 10, "TGA image format not supported");

		const u32 channels = depth / 8;

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
		u32 magic;
		br.read(magic);
		CE_ASSERT(magic == DDSD_MAGIC, "DDS bad magic number");

		u32 hsize;
		br.read(hsize);
		CE_ASSERT(hsize == DDSD_HEADERSIZE, "DDS bas header size");

		u32 flags;
		br.read(flags);
		CE_ASSERT(flags & (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT), "DDS bad header flags");

		u32 height;
		br.read(height);

		u32 width;
		br.read(width);

		u32 pitch;
		br.read(pitch);

		u32 depth;
		br.read(depth);

		u32 num_mips;
		br.read(num_mips);

		// Skip reserved bits
		br.skip(sizeof(u32) * 11);

		// Read pixel format
		u32 pf_hsize;
		br.read(pf_hsize);
		CE_ASSERT(pf_hsize == DDPF_HEADERSIZE, "DDS bad pf header size");

		u32 pf_flags;
		br.read(pf_flags);

		u32 pf_fourcc;
		br.read(pf_fourcc);

		u32 pf_bitcount;
		br.read(pf_bitcount);

		u32 pf_rmask;
		br.read(pf_rmask);

		u32 pf_gmask;
		br.read(pf_gmask);

		u32 pf_bmask;
		br.read(pf_bmask);

		u32 pf_amask;
		br.read(pf_amask);

		u32 caps;
		br.read(caps);
		CE_ASSERT((caps & DDSCAPS_TEXTURE), "DDS bad caps");

		u32 caps2;
		br.read(caps2);

		u32 caps3;
		br.read(caps3);

		u32 caps4;
		br.read(caps4);

		u32 reserved2;
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

		const u32 raw_fmt = (pf_flags & DDPF_FOURCC) ? pf_fourcc : pf_flags;
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

	void write_dds(const ImageData& image, CompileOptions& opts)
	{
		opts.write(DDSD_MAGIC);

		// Header
		opts.write(DDSD_HEADERSIZE); // dwSize
		opts.write(DDS_HEADER_FLAGS_TEXTURE
			| DDSD_MIPMAPCOUNT
			| (pixel_format::is_compressed(image.format) ? DDSD_LINEARSIZE : DDSD_PITCH)
			| (image.num_mips ? DDSD_MIPMAPCOUNT : 0)); // dwFlags
		opts.write(image.height); // dwHeight
		opts.write(image.width); // dwWidth

		const u32 pitch = pixel_format::is_compressed(image.format) ? 0 // fixme
								: (image.width * pixel_format::size(image.format) * 8 + 7) / 8;

		opts.write(pitch); // dwPitchOrLinearSize
		opts.write(DDSD_UNUSED); // dwDepth
		opts.write(image.num_mips); // dwMipMapCount;

		for (u32 i = 0; i < 11; i++)
			opts.write(DDSD_UNUSED); // dwReserved1[11];

		// Pixel format
		opts.write(DDPF_HEADERSIZE); // dwSize;
		u32 pf = 0;
		switch (image.format)
		{
			case PixelFormat::DXT1:     pf = DDPF_FOURCC_DXT1; break;
			case PixelFormat::DXT3:     pf = DDPF_FOURCC_DXT3; break;
			case PixelFormat::DXT5:     pf = DDPF_FOURCC_DXT5; break;
			case PixelFormat::R8G8B8:   pf = DDS_RGB; break;
			case PixelFormat::R8G8B8A8: pf = DDS_RGBA; break;
			default: CE_FATAL("Pixel format unknown"); break;
		}
		opts.write(pixel_format::is_compressed(image.format) ? DDPF_FOURCC : pf); // dwFlags;
		opts.write(pixel_format::is_compressed(image.format) ? pf : DDSD_UNUSED); // dwFourCC;
		opts.write(u32(pixel_format::size(image.format) * 8)); // dwRGBBitCount;
		opts.write(u32(0x00ff0000)); // dwRBitMask;
		opts.write(u32(0x0000ff00)); // dwGBitMask;
		opts.write(u32(0x000000ff)); // dwBBitMask;
		opts.write(u32(0xff000000)); // dwABitMask;

		opts.write(DDSCAPS_TEXTURE
			| (image.num_mips > 1 ? DDSCAPS_COMPLEX : DDSD_UNUSED) // also for cubemap, depth mipmap
			| (image.num_mips > 1 ? DDSCAPS_MIPMAP : DDSD_UNUSED)); // dwCaps;
		opts.write(DDSD_UNUSED); // dwCaps2;
		opts.write(DDSD_UNUSED); // dwCaps3;
		opts.write(DDSD_UNUSED); // dwCaps4;
		opts.write(DDSD_UNUSED); // dwReserved2;

		// Image data
		for (u32 i = 0; i < image.num_mips; i++)
		{
			MipData mip;
			read_mip_image(image, i, mip);

			// CE_LOGD("Writing mip: (%ux%u) byes = %u", mip.width, mip.height, mip.size);
			opts.write(mip.data, mip.size);
		}
	}

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		DynamicString name(ta);
		sjson::parse_string(object["source"], name);

		File* source = opts._fs.open(name.c_str(), FileOpenMode::READ);
		BinaryReader br(*source);
		ImageData image;

		if (name.ends_with(".tga"))
		{
			parse_tga(br, image);
		}
		else if (name.ends_with(".dds"))
		{
			// parse_dds(br, image);
			// u32 size = source->size();
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

		opts._fs.close(*source);

		// Write DDS
		opts.write(TEXTURE_VERSION); // Version
		opts.write(u32(0)); // Size
		write_dds(image, opts);

		default_allocator().deallocate(image.data);
	}

	void* load(File& file, Allocator& a)
	{
		const u32 file_size = file.size();
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
