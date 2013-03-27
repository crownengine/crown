#include "TextResource.h"
#include "FileStream.h"
#include "ResourceArchive.h"
#include "Log.h"

#include <cstdio>

namespace crown
{

//-----------------------------------------------------------------------------
TextResource* TextResource::load(ResourceArchive* archive, ResourceId id)
{
	assert(archive != NULL);
	
	Log::D("TextResource::load called.");
	
	FileStream* stream = archive->find(id);

	if (stream != NULL)
	{
		TextResource* resource = new TextResource;

		stream->read(&resource->length, sizeof(uint32_t));
		
		resource->data = new char[resource->length + 1];
		
		printf("Resource length: %d\n", resource->length);
		
		stream->read(resource->data, (size_t)resource->length);
		
		resource->data[resource->length] = '\0';

		return resource;
	}

	Log::E("Unable to find the resource.");

	return NULL;
}

//-----------------------------------------------------------------------------
void TextResource::unload(TextResource* text)
{

}

} // namespace crown

