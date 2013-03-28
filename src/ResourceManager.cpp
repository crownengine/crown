/*
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

#include "Types.h"
#include "ResourceManager.h"
#include "ResourceLoader.h"
#include "String.h"
#include "Hash.h"
#include "Path.h"
#include <algorithm>

namespace crown
{

//-----------------------------------------------------------------------------
ResourceManager::ResourceManager(Filesystem* filesystem) :
	m_resource_loader(this, filesystem),
	m_resources(m_allocator)
{
}

//-----------------------------------------------------------------------------
ResourceManager::~ResourceManager()
{
}

//-----------------------------------------------------------------------------
ResourceId ResourceManager::load(const char* name)
{
	char basename[512];
	char extension[512];

	path::filename_without_extension(name, basename, 512);
	path::extension(name, extension, 512);

	uint32_t name_hash = hash::fnv1a_32(basename, string::strlen(basename));
	uint32_t type_hash = hash::fnv1a_32(extension, string::strlen(extension));

	return load(name_hash, type_hash);
}

//-----------------------------------------------------------------------------
ResourceId ResourceManager::load(uint32_t name, uint32_t type)
{
	// Search for an already existent resource
	ResourceEntry* entry = std::find(m_resources.begin(), m_resources.end(), name);

	// If resource not found, create a new one
	if (entry == m_resources.end())
	{
		ResourceId id;
		id.name = name;
		id.type = type;
		id.index = m_resources.size();

		ResourceEntry entry;
		entry.id = id;
		entry.state = RS_UNLOADED;
		entry.references = 1;
		entry.resource = NULL;

		m_resources.push_back(entry);

		m_resource_loader.load(id);

		return id;
	}

	// Else, increment its reference count
	entry->references++;
	
	return entry->id;
}

//-----------------------------------------------------------------------------
void ResourceManager::unload(ResourceId name)
{
	assert(has(name));
	
	ResourceEntry& entry = m_resources[name.index];
	
	entry.references--;
	
	if (entry.references == 0)
	{
		entry.state = RS_UNLOADED;
		entry.resource = NULL;

		m_resource_loader.unload(name);
	}
}

//-----------------------------------------------------------------------------
void ResourceManager::reload(ResourceId name)
{
	assert(has(name));
	
	ResourceEntry& entry = m_resources[name.index];
	
	if (entry.state == RS_LOADED)
	{
		// FIXME
	}
}

//-----------------------------------------------------------------------------
bool ResourceManager::has(ResourceId name)
{
	if (m_resources.size() > name.index)
	{
		return (m_resources[name.index].id.name == name.name);
	}
	
	return false;
}

//-----------------------------------------------------------------------------
void* ResourceManager::data(ResourceId name)
{
	assert(has(name));
	
	return m_resources[name.index].resource;
}

//-----------------------------------------------------------------------------
bool ResourceManager::is_loaded(ResourceId name)
{
	assert(has(name));

	return m_resources[name.index].state == RS_LOADED;
}

//-----------------------------------------------------------------------------
void ResourceManager::loading(ResourceId name)
{
	assert(has(name));

	m_resources[name.index].state = RS_LOADING;
}

//-----------------------------------------------------------------------------
void ResourceManager::online(ResourceId name, void* resource)
{
	assert(has(name));

	ResourceEntry& entry = m_resources[name.index];

	entry.resource = resource;
	entry.state = RS_LOADED;
}

} // namespace crown

