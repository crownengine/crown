#include "TextureResource.h"
#include "ResourceArchive.h"
#include "Log.h"
#include "FileStream.h"
#include <cassert>
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
TextureResource* TextureResource::load(Allocator& allocator, ResourceArchive* archive, ResourceId id)
{
	assert(archive != NULL);
	
	FileStream* stream = archive->find(id);

	if (stream != NULL)
	{
		TextureResource* resource = (TextureResource*)allocator.allocate(sizeof(TextureResource));
	
		stream->read(&resource->format, sizeof(PixelFormat));
		stream->read(&resource->width, sizeof(uint16_t));
		stream->read(&resource->height, sizeof(uint16_t));
		
		stream->read(&resource->mode, sizeof(TextureMode));
		stream->read(&resource->filter, sizeof(TextureFilter));
		stream->read(&resource->wrap, sizeof(TextureWrap));
	
		size_t size = resource->width * resource->height * Pixel::GetBytesPerPixel(resource->format);

		resource->data = (uint8_t*)allocator.allocate(sizeof(uint8_t) * size);

		stream->read(resource->data, size);

		return resource;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void TextureResource::unload(Allocator& allocator, TextureResource* resource)
{
	assert(resource != NULL);

	allocator.deallocate(resource->data);
	allocator.deallocate(resource);
}

} // namespace crown
