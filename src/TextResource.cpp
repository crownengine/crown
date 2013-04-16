#include "TextResource.h"
#include "FileStream.h"
#include "ResourceArchive.h"
#include "Log.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
TextResource* TextResource::load(Allocator& allocator, ResourceArchive* archive, ResourceId id)
{
	assert(archive != NULL);
	
	FileStream* stream = archive->find(id);

	if (stream != NULL)
	{
		TextResource* resource = (TextResource*)allocator.allocate(sizeof(TextResource));

		stream->read(&resource->length, sizeof(uint32_t));
		
		resource->data = (char*)allocator.allocate(sizeof(char) * (resource->length + 1));

		stream->read(resource->data, (size_t)resource->length);
		
		resource->data[resource->length] = '\0';

		return resource;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void TextResource::unload(Allocator& allocator, TextResource* text)
{
	assert(text != NULL);

	text->length = 0;

	allocator.deallocate(text->data);
	allocator.deallocate(text);
}

} // namespace crown
