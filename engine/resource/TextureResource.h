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
#include "Resource.h"
#include "PixelFormat.h"
#include "Bundle.h"
#include "Allocator.h"
#include "File.h"
#include "Device.h"
#include "Renderer.h"

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

#define DDFP_FOURCC_DXT1			FOURCC('D', 'X', 'T', '1')
#define DDFP_FOURCC_DXT2			FOURCC('D', 'X', 'T', '2')
#define DDFP_FOURCC_DXT3			FOURCC('D', 'X', 'T', '3')
#define DDFP_FOURCC_DXT4			FOURCC('D', 'X', 'T', '4')
#define DDFP_FOURCC_DXT5			FOURCC('D', 'X', 'T', '5')
#define DDFP_FOURCC_DX10			FOURCC('D', 'X', '1', '0')

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

struct TextureHeader
{
	TextureId id;
};

#define DDS_HEADER_OFFSET	uint32_t(sizeof(TextureHeader))
#define DDS_DATA_OFFSET		uint32_t(DDS_HEADER_OFFSET + DDSD_HEADERSIZE)

struct TextureResource
{
	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
		TextureResource* t = (TextureResource*) resource;
		TextureHeader* h = (TextureHeader*) t;

		h->id = device()->renderer()->create_texture(t->width(), t->height(), t->num_mipmaps(), t->format(), t->data());
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT(resource != NULL, "Resource not loaded");

		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
		TextureResource* t = (TextureResource*) resource;

		device()->renderer()->destroy_texture(t->texture());
	}

	PixelFormat::Enum format() const
	{
		const DdsPixelFormat& ddspf = ((DdsHeader*) (((char*) this) + DDS_HEADER_OFFSET))->ddspf;

		const uint32_t fmt = ddspf.flags & DDPF_FOURCC ? ddspf.fourcc : ddspf.flags;
		switch (fmt)
		{
			case DDFP_FOURCC_DXT1: return PixelFormat::DXT1; break;
			case DDFP_FOURCC_DXT3: return PixelFormat::DXT3; break;
			case DDFP_FOURCC_DXT5: return PixelFormat::DXT5; break;
			case DDS_RGB: return PixelFormat::R8G8B8; break;
			case DDS_RGBA: return PixelFormat::R8G8B8A8; break;
			default: CE_FATAL("Unknown pixel format"); return PixelFormat::COUNT;
		}
	}

	uint32_t width() const
	{
		return ((DdsHeader*) (((char*) this) + DDS_HEADER_OFFSET))->width;
	}

	uint32_t height() const
	{
		return ((DdsHeader*) (((char*) this) + DDS_HEADER_OFFSET))->height;
	}

	uint8_t num_mipmaps() const
	{
		return (uint8_t) ((DdsHeader*) (((char*) this) + DDS_HEADER_OFFSET))->num_mips;
	}

	const char* data() const
	{
		return (char*) this + DDS_DATA_OFFSET;
	}

	TextureId texture() const
	{
		return ((TextureHeader*) this)->id;
	}

private:

	// Disable construction
	TextureResource();
};

} // namespace crown
