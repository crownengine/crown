#include "TextureResource.h"
#include "ResourceArchive.h"
#include "Log.h"
#include "FileStream.h"
#include "Assert.h"
#include "Allocator.h"
#include "Device.h"
#include "Renderer.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* TextureResource::load(Allocator& allocator, ResourceArchive& archive, ResourceId id)
{
	FileStream* stream = archive.open(id);

	CE_ASSERT(stream != NULL, "Resource does not exist: %.8X%.8X", id.name, id.type);

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
