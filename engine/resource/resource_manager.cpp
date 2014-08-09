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

#include <algorithm>
#include <inttypes.h>
#include "types.h"
#include "resource_manager.h"
#include "resource_registry.h"
#include "string_utils.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "dynamic_string.h"
#include "queue.h"
#include "log.h"

namespace crown
{

ResourceId::ResourceId(const char* type, const char* name)
	: type(string::murmur2_64(type, string::strlen(type), 0))
	, name(string::murmur2_64(name, string::strlen(name), 0))
{
}

//-----------------------------------------------------------------------------
ResourceManager::ResourceManager(Bundle& bundle)
	: m_resource_heap("resource", default_allocator())
	, m_loader(bundle, m_resource_heap)
	, m_resources(default_allocator())
{
}

//-----------------------------------------------------------------------------
void ResourceManager::load(ResourceId id)
{
	// Search for an already existent resource
	ResourceEntry* entry = find(id);

	// If resource not found, post load request
	if (entry == NULL)
	{
		m_loader.load(id);
		return;
	}

	// Else, increment its reference count
	entry->references++;
}

//-----------------------------------------------------------------------------
void ResourceManager::unload(ResourceId id, bool force)
{
	CE_ASSERT(find(id) != NULL, "Resource not loaded: ""%.16"PRIx64"-%.16"PRIx64, id.type, id.name);

	flush();
	ResourceEntry* entry = find(id);
	entry->references--;

	if (entry->references == 0 || force)
	{
		resource_on_offline(id.type, entry->resource);
		resource_on_unload(id.type, m_resource_heap, entry->resource);

		// Swap with last
		ResourceEntry temp = m_resources[array::size(m_resources) - 1];
		(*entry) = temp;
		array::pop_back(m_resources);
	}
}

//-----------------------------------------------------------------------------
bool ResourceManager::can_get(ResourceId id) const
{
	return find(id) != NULL;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::get(const char* type, const char* name) const
{
	ResourceEntry* entry = find(ResourceId(type, name));
	CE_ASSERT(entry != NULL, "Resource not loaded: %s.%s", name, type);
	return entry->resource;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::get(ResourceId id) const
{
	CE_ASSERT(find(id) != NULL, "Resource not loaded: ""%.16"PRIx64"-%.16"PRIx64, id.type, id.name);
	return find(id)->resource;
}

//-----------------------------------------------------------------------------
uint32_t ResourceManager::references(ResourceId id) const
{
	CE_ASSERT(find(id) != NULL, "Resource not loaded: ""%.16"PRIx64"-%.16"PRIx64, id.type, id.name);
	return find(id)->references;
}

//-----------------------------------------------------------------------------
void ResourceManager::flush()
{
	m_loader.flush();
	complete_requests();
}

//-----------------------------------------------------------------------------
ResourceEntry* ResourceManager::find(ResourceId id) const
{
	const ResourceEntry* entry = std::find(array::begin(m_resources), array::end(m_resources), id);
	return entry != array::end(m_resources) ? const_cast<ResourceEntry*>(entry) : NULL;
}

//-----------------------------------------------------------------------------
void ResourceManager::complete_requests()
{
	TempAllocator1024 ta;
	Array<ResourceData> loaded(ta);
	m_loader.get_loaded(loaded);

	for (uint32_t i = 0; i < array::size(loaded); i++)
		complete_request(loaded[i].id, loaded[i].data);
}

//-----------------------------------------------------------------------------
void ResourceManager::complete_request(ResourceId id, void* data)
{
	resource_on_online(id.type, data);
	ResourceEntry entry;
	entry.id = id;
	entry.references = 1;
	entry.resource = data;
	array::push_back(m_resources, entry);
}

} // namespace crown
