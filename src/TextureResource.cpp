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

#include "TextureResource.h"
#include "Bundle.h"
#include "Log.h"
#include "DiskFile.h"
#include "Assert.h"
#include "Allocator.h"
#include "Device.h"
#include "Renderer.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* TextureResource::load(Allocator& allocator, Bundle& bundle, ResourceId id)
{
	DiskFile* file = bundle.open(id);

	CE_ASSERT(file != NULL, "Resource does not exist: %.8X%.8X", id.name, id.type);

	TextureResource* resource = (TextureResource*)allocator.allocate(sizeof(TextureResource));

	file->read(&resource->m_header, sizeof(TextureHeader));

	size_t size = resource->width() * resource->height() * Pixel::bytes_per_pixel(resource->format());

	resource->m_data = (uint8_t*)allocator.allocate(sizeof(uint8_t) * size);

	file->read(resource->m_data, size);

	bundle.close(file);

	return resource;
}

//-----------------------------------------------------------------------------
void TextureResource::online(void* resource)
{
	CE_ASSERT(resource != NULL, "Resource not loaded");
}

//-----------------------------------------------------------------------------
void TextureResource::unload(Allocator& allocator, void* resource)
{
	CE_ASSERT(resource != NULL, "Resource not loaded");

	allocator.deallocate(((TextureResource*)resource)->m_data);
	allocator.deallocate(resource);
}

//-----------------------------------------------------------------------------
void TextureResource::offline()
{

}

} // namespace crown
