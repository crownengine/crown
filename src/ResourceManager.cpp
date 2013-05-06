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
#include "String.h"
#include "Hash.h"
#include "Path.h"
#include "Log.h"
#include <algorithm>

#include "TextResource.h"
#include "TextureResource.h"
#include "ScriptResource.h"

namespace crown
{

//-----------------------------------------------------------------------------
ResourceManager::ResourceManager(ResourceArchive& archive, Allocator& allocator) :
	m_resource_archive(archive),
	m_resource_allocator(allocator),
	m_resources(m_allocator),
	m_loading_queue(m_allocator),
	m_loaded_queue(m_allocator),
	m_thread(ResourceManager::background_thread, (void*)this, "resource-loader-thread")
{
	// FIXME hardcoded seed
	m_texture_hash = hash::murmur2_32("tga", 3, 0);
	m_mesh_hash = hash::murmur2_32("mesh", 4, 0);
	m_txt_hash = hash::murmur2_32("txt", 3, 0);
	m_script_hash = hash::murmur2_32("lua", 3, 0);
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

	// FIXME hardcoded seed
	uint32_t name_hash = hash::murmur2_32(basename, string::strlen(basename), 0);
	uint32_t type_hash = hash::murmur2_32(extension, string::strlen(extension), 0);

	return load(name_hash, type_hash);
}

//-----------------------------------------------------------------------------
void ResourceManager::unload(ResourceId name)
{
	assert(has(name));
	
	m_resources_mutex.lock();

	ResourceEntry& entry = m_resources[name.index];
	
	entry.references--;
	
	if (entry.references == 0 && entry.state == RS_LOADED)
	{
		unload_by_type(name, entry.resource);

		entry.state = RS_UNLOADED;
		entry.resource = NULL;
	}

	m_resources_mutex.unlock();
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
bool ResourceManager::has(ResourceId name) const
{
	bool has_resource = false;

	m_resources_mutex.lock();

	if (m_resources.size() > name.index)
	{
		 has_resource = (m_resources[name.index].id.name == name.name);
	}

	m_resources_mutex.unlock();
	
	return has_resource;
}

//-----------------------------------------------------------------------------
const void* ResourceManager::data(ResourceId name) const
{
	assert(has(name));
	
	m_resources_mutex.lock();

	void* resource = m_resources[name.index].resource;

	m_resources_mutex.unlock();

	return resource;
}

//-----------------------------------------------------------------------------
bool ResourceManager::is_loaded(ResourceId name) const
{
	assert(has(name));

	m_resources_mutex.lock();

	bool loaded = m_resources[name.index].state == RS_LOADED;

	m_resources_mutex.unlock();

	return loaded;
}

//-----------------------------------------------------------------------------
uint32_t ResourceManager::references(ResourceId name) const
{
	assert(has(name));

	m_resources_mutex.lock();

	bool loaded = m_resources[name.index].references;

	m_resources_mutex.unlock();

	return loaded;
}

//-----------------------------------------------------------------------------
void ResourceManager::flush_load_queue()
{
	m_loading_mutex.lock();

	if (m_loading_queue.size() > 0)
	{
		m_loading_requests.signal();
	}
	
	m_loading_mutex.unlock();
}

//-----------------------------------------------------------------------------
void ResourceManager::bring_loaded_online()
{
	m_loaded_mutex.lock();

	// Update master table and bring online
	while (m_loaded_queue.size() > 0)
	{
		LoadedResource lr = m_loaded_queue.front();
		m_loaded_queue.pop_front();

		online(lr.resource, lr.data);
	}

	m_loaded_mutex.unlock();
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

		m_loading_mutex.lock();
		m_loading_queue.push_back(id);
		m_loading_mutex.unlock();

		return id;
	}

	// Else, increment its reference count
	entry->references++;
	
	return entry->id;
}

//-----------------------------------------------------------------------------
void ResourceManager::background_load()
{
	// FIXME: Maybe epic crash because of concurrent access to the same allocator?
	while (true)
	{
		m_loading_mutex.lock();
		m_loading_requests.wait(m_loading_mutex);

		ResourceId resource = m_loading_queue.front();
		m_loading_queue.pop_front();

		m_loading_mutex.unlock();

		void* data = load_by_type(resource);

		LoadedResource lr;
		lr.resource = resource;
		lr.data = data;

		m_loaded_mutex.lock();

		m_loaded_queue.push_back(lr);

		m_loaded_mutex.unlock();
	}
}

//-----------------------------------------------------------------------------
void* ResourceManager::load_by_type(ResourceId name) const
{
	if (name.type == m_texture_hash)
	{
		return TextureResource::load(m_resource_allocator, m_resource_archive, name);
	}
	else if (name.type == m_txt_hash)
	{
		return TextResource::load(m_resource_allocator, m_resource_archive, name);
	}
	else if (name.type == m_script_hash)
	{
		return ScriptResource::load(m_resource_allocator, m_resource_archive, name);
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void ResourceManager::unload_by_type(ResourceId name, void* resource) const
{
	if (name.type == m_texture_hash)
	{
		TextureResource::unload(m_resource_allocator, (TextureResource*)resource);
	}
	else if (name.type == m_txt_hash)
	{
		TextResource::unload(m_resource_allocator, (TextResource*)resource);
	}
	else if (name.type == m_script_hash)
	{
		ScriptResource::unload(m_resource_allocator, (ScriptResource*)resource);
	}

	return;
}

//-----------------------------------------------------------------------------
void ResourceManager::online(ResourceId name, void* resource)
{
	if (name.type == m_texture_hash)
	{
		TextureResource::online((TextureResource*)resource);
	}
	else if (name.type == m_txt_hash)
	{
		TextResource::unload(m_resource_allocator, (TextResource*)resource);
	}
	else if (name.type == m_script_hash)
	{
		ScriptResource::online((ScriptResource*)resource);
	}

	m_resources_mutex.lock();

	ResourceEntry& entry = m_resources[name.index];
	entry.resource = resource;
	entry.state = RS_LOADED;

	m_resources_mutex.unlock();
}

//-----------------------------------------------------------------------------
void* ResourceManager::background_thread(void* thiz)
{
	ResourceManager* mgr = (ResourceManager*)thiz;

	mgr->background_load();
}

} // namespace crown
