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

// Bump the version whenever a change in the header is made
const uint32_t TEXTURE_VERSION = 1;

struct TextureHeader
{
	uint32_t	version;	// Texture file version
	uint32_t	format;		// Format of the pixels
	uint32_t	width;		// Width in pixels
	uint32_t	height;		// Height in pixels
};

class TextureResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);

		TextureResource* resource = (TextureResource*)allocator.allocate(sizeof(TextureResource));

		file->read(&resource->m_header, sizeof(TextureHeader));

		size_t size = resource->width() * resource->height() * Pixel::bytes_per_pixel(resource->format());

		resource->m_data = (uint8_t*)allocator.allocate(sizeof(uint8_t) * size);

		file->read(resource->m_data, size);

		bundle.close(file);

		return resource;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
		TextureResource* t = (TextureResource*) resource;

		t->m_texture = device()->renderer()->create_texture(t->width(), t->height(), t->format(), t->data());
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT(resource != NULL, "Resource not loaded");

		allocator.deallocate(((TextureResource*)resource)->m_data);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
		TextureResource* t = (TextureResource*) resource;

		device()->renderer()->destroy_texture(t->m_texture);
	}

public:

	PixelFormat::Enum format() const
	{
		return (PixelFormat::Enum) m_header.format;
	}

	uint32_t width() const
	{
		return m_header.width;
	}

	uint32_t height() const
	{
		return m_header.height;
	}

	const uint8_t* data() const
	{
		return m_data;
	}

public:

	TextureHeader		m_header;
	uint8_t*			m_data;

	TextureId m_texture;
};

} // namespace crown
