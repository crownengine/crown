#include "TextureResource.h"
#include "ResourceArchive.h"
#include "Log.h"
#include "FileStream.h"
#include <cassert>

namespace crown
{

//-----------------------------------------------------------------------------
TextureResource* TextureResource::load(ResourceArchive* archive, ResourceId id)
{
	assert(archive != NULL);
	
	Log::D("TextureResource::load called.");
	
	FileStream* stream = archive->find(id);

	if (stream != NULL)
	{
		TextureResource* resource = new TextureResource;
	
		stream->read(&resource->format, sizeof(PixelFormat));
		stream->read(&resource->width, sizeof(uint16_t));
		stream->read(&resource->height, sizeof(uint16_t));
		
		stream->read(&resource->mode, sizeof(TextureMode));
		stream->read(&resource->filter, sizeof(TextureFilter));
		stream->read(&resource->wrap, sizeof(TextureWrap));
		
		printf("Debug: Format = %d\n", resource->format);
		printf("Debug: Width  = %d\n", resource->width);
		printf("Debug: Height = %d\n", resource->height);
		printf("Debug: Mode   = %d\n", resource->mode);
		printf("Debug: Filter = %d\n", resource->filter);
		printf("Debug: Wrap   = %d\n", resource->wrap);
	
		size_t size = resource->width * resource->height * Pixel::GetBytesPerPixel(resource->format);

		resource->data = new uint8_t[size];

		stream->read(resource->data, size);

		return resource;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void TextureResource::unload(TextureResource* resource)
{
	// TODO
}
	
} // namespace crown

