#include "TextResource.h"
#include "FileStream.h"
#include "ResourceArchive.h"
#include "Log.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* TextResource::load(Allocator& allocator, ResourceArchive& archive, ResourceId id)
{
	FileStream* stream = archive.open(id);

	if (stream != NULL)
	{
		TextResource* resource = (TextResource*)allocator.allocate(sizeof(TextResource));

		stream->read(&resource->length, sizeof(uint32_t));
		
		resource->data = (char*)allocator.allocate(sizeof(char) * (resource->length + 1));

		stream->read(resource->data, (size_t)resource->length);
		
		resource->data[resource->length] = '\0';

		archive.close(stream);

		return resource;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void TextResource::unload(Allocator& allocator, void* resource)
{
	assert(resource != NULL);

	((TextResource*)resource)->length = 0;

	allocator.deallocate(((TextResource*)resource)->data);
	allocator.deallocate(resource);
}

} // namespace crown
