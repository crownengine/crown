#include <cassert>

#include "ScriptResource.h"
#include "ResourceArchive.h"
#include "Log.h"
#include "FileStream.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* ScriptResource::load(Allocator& allocator, ResourceArchive& archive, ResourceId id)
{
	FileStream* stream = archive.open(id);

	if (stream != NULL)
	{
		ScriptResource* resource = (ScriptResource*)allocator.allocate(sizeof(ScriptResource));
	
		size_t size = stream->size() - stream->position();

		resource->m_data = (uint8_t*)allocator.allocate(sizeof(uint8_t) * size);

		stream->read(resource->m_data, size);

		archive.close(stream);

		return resource;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void ScriptResource::online(void* resource)
{
	(void) resource;
}

//-----------------------------------------------------------------------------
void ScriptResource::unload(Allocator& allocator, void* resource)
{
	assert(resource != NULL);

	allocator.deallocate(((ScriptResource*)resource)->m_data);
	allocator.deallocate(resource);
}

//-----------------------------------------------------------------------------
void ScriptResource::offline()
{

}

} // namespace crown
