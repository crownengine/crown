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
#include "Types.h"
#include "ResourceManager.h"
#include "ResourceRegistry.h"
#include "StringUtils.h"
#include "StringUtils.h"
#include "TempAllocator.h"
#include "DynamicString.h"
#include "Queue.h"

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
	, m_pendings(default_allocator())
	, m_resources(default_allocator())
{
	m_loader.start();
}

//-----------------------------------------------------------------------------
ResourceManager::~ResourceManager()
{
	m_loader.stop();
}

//-----------------------------------------------------------------------------
void ResourceManager::load(ResourceId id)
{
	// Search for an already existent resource
	ResourceEntry* entry = find(id);

	// If resource not found, create a new one
	if (entry == NULL)
	{
		ResourceEntry entry;

		entry.id = id;
		entry.references = 1;
		entry.resource = NULL;

		array::push_back(m_resources, entry);

		// Issue request to resource loader
		PendingRequest pr;
		pr.resource = id;
		pr.id = m_loader.load_resource(id);

		queue::push_back(m_pendings, pr);

		return;
	}

	// Else, increment its reference count
	entry->references++;
}

//-----------------------------------------------------------------------------
void ResourceManager::unload(ResourceId id, bool force)
{
	CE_ASSERT(has(id), "Resource not loaded: %.16lx-%16lx", id.type, id.name);

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
bool ResourceManager::has(ResourceId id) const
{
	ResourceEntry* entry = find(id);

	return entry != NULL;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::get(const char* type, const char* name) const
{
	ResourceEntry* entry = find(ResourceId(type, name));

	CE_ASSERT(entry != NULL, "Resource not loaded: type = '%s', name = '%s'", type, name);
	CE_ASSERT(entry->resource != NULL, "Resource not loaded: type = '%s', name = '%s'", type, name);

	return entry->resource;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::get(ResourceId id) const
{
	CE_ASSERT(has(id), "Resource not loaded: %.16lx-%16lx", id.type, id.name);

	return find(id)->resource;
}

//-----------------------------------------------------------------------------
bool ResourceManager::is_loaded(ResourceId id) const
{
	CE_ASSERT(has(id), "Resource not loaded: %.16lx-%16lx", id.type, id.name);

	return find(id)->resource != NULL;
}

//-----------------------------------------------------------------------------
uint32_t ResourceManager::references(ResourceId id) const
{
	CE_ASSERT(has(id), "Resource not loaded: %.16lx-%16lx", id.type, id.name);

	return find(id)->references;
}

//-----------------------------------------------------------------------------
void ResourceManager::flush()
{
	while (!queue::empty(m_pendings))
	{
		poll_resource_loader();
	}
}

//-----------------------------------------------------------------------------
ResourceEntry* ResourceManager::find(ResourceId id) const
{
	const ResourceEntry* entry = std::find(array::begin(m_resources), array::end(m_resources), id);

	return entry != array::end(m_resources) ? const_cast<ResourceEntry*>(entry) : NULL;
}

//-----------------------------------------------------------------------------
void ResourceManager::poll_resource_loader()
{
	if (!queue::empty(m_pendings))
	{
		PendingRequest request = queue::front(m_pendings);

		if (m_loader.load_resource_status(request.id) == LRS_LOADED)
		{
			queue::pop_front(m_pendings);

			void* data = m_loader.load_resource_data(request.id);
			online(request.resource, data);
		}
	}
}

//-----------------------------------------------------------------------------
void ResourceManager::online(ResourceId id, void* resource)
{
	ResourceEntry* entry = find(id);
	resource_on_online(id.type, resource);

	entry->resource = resource;
}

} // namespace crown
