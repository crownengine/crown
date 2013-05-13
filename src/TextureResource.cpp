#include "TextureResource.h"
#include "ResourceArchive.h"
#include "Log.h"
#include "FileStream.h"
#include <cassert>
#include "Allocator.h"
#include "Device.h"
#include "Renderer.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* TextureResource::load(Allocator& allocator, ResourceArchive& archive, ResourceId id)
{
	FileStream* stream = archive.open(id);

	if (stream != NULL)
	{
		TextureResource* resource = (TextureResource*)allocator.allocate(sizeof(TextureResource));
	
		stream->read(&resource->m_format, sizeof(PixelFormat));
		stream->read(&resource->m_width, sizeof(uint16_t));
		stream->read(&resource->m_height, sizeof(uint16_t));
	
		size_t size = resource->m_width * resource->m_height * Pixel::bytes_per_pixel(resource->m_format);

		resource->m_data = (uint8_t*)allocator.allocate(sizeof(uint8_t) * size);

		stream->read(resource->m_data, size);

		archive.close(stream);

		return resource;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void TextureResource::online(void* resource)
{
	assert(resource != NULL);
}

//-----------------------------------------------------------------------------
void TextureResource::unload(Allocator& allocator, void* resource)
{
	assert(resource != NULL);

	allocator.deallocate(((TextureResource*)resource)->m_data);
	allocator.deallocate(resource);
}

//-----------------------------------------------------------------------------
void TextureResource::offline()
{

}

} // namespace crown
