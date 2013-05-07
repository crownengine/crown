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

#pragma once

#include "Types.h"
#include "List.h"
#include "Queue.h"
#include "Resource.h"
#include "MallocAllocator.h"
#include "Thread.h"
#include "Mutex.h"
#include "Cond.h"

namespace crown
{

struct ResourceEntry
{
	ResourceId		id;
	ResourceState	state;

	uint32_t		references;

	void*			resource;

	bool			operator==(const uint32_t& name)
					{
						return id.name == name;
					}

	bool			operator==(const ResourceEntry& b)
					{
						return id.name == b.id.name;
					}
};

struct LoadedResource
{
	ResourceId	resource;
	void*		data;
};

class ResourceArchive;

/// Resource manager.
class ResourceManager
{
public:

	/// Read resources from @archive and store resource data using @allocator.
							ResourceManager(ResourceArchive& archive, Allocator& allocator);
							~ResourceManager();

	/// Loads the resource by @name and returns its ResourceId.
	/// @note
	/// The resource data may be not immediately available,
	/// the resource gets pushed in a queue of load requests and loadead as
	/// soon as possible by the ResourceLoader.
	/// You have to explicitly call is_loaded() method to check if the
	/// loading process is actually completed.
	ResourceId				load(const char* name);

	/// Unloads the @resource, freeing up all the memory associated by it
	/// and eventually any global object associated with it.
	/// (Such as texture objects, vertex buffers etc.)
	void					unload(ResourceId name);

	/// Reloads the @resource
	void					reload(ResourceId name);

	/// Returns whether the manager has the @name resource into
	/// its list of resources.
	/// @note
	/// Having a resource does not mean that the resource is
	/// available for using; instead, you have to check is_loaded() to
	/// obtain the resource availability status.
	bool					has(ResourceId name) const;

	/// Returns the data associated with the @name resource.
	/// The resource data contains resource-specific metadata
	/// and the actual resource data. In order to correctly use
	/// it, you have to know which type of data @name refers to
	/// and cast accordingly.
	const void*				data(ResourceId name) const;
	
	/// Returns whether the @name resource is loaded (i.e. whether
	/// you can use the data associated with it).
	bool					is_loaded(ResourceId name) const;

	/// Returns the number of references to the @resource
	uint32_t				references(ResourceId name) const;

	/// Returns the number of resources still waiting to load.
	uint32_t				remaining() const;

	/// Forces all the loading requests to complete before preceeding.
	void					flush();

private:

	// Checks the load queue and signal the backgroud about pending
	// requests. It is normally called only by the Device.
	void					check_load_queue();
	// Calls online() on loaded resources. Must be called only
	// in the main thread and generally only by Device.
	void					bring_loaded_online();

	// Loads the resource by name and type and returns its ResourceId.
	ResourceId				load(uint32_t name, uint32_t type);

	void					background_load();

	void*					load_by_type(ResourceId name) const;
	void					unload_by_type(ResourceId name, void* resource) const;
	void					online(ResourceId name, void* resource);

private:

	static void*			background_thread(void* thiz);

private:

	// Archive whether to look for resources
	ResourceArchive&		m_resource_archive;
	// Used to strore resource memory
	Allocator&				m_resource_allocator;

	MallocAllocator			m_allocator;
	// The master lookup table
	List<ResourceEntry>		m_resources;

	// Resources waiting for loading
	Queue<ResourceId>		m_loading_queue;
	// Resources already loaded, ready to bring online
	Queue<LoadedResource>	m_loaded_queue;

	// Background loading thread
	Thread					m_thread;

	mutable Mutex			m_loading_mutex;
	Cond 					m_loading_requests;
	Cond 					m_all_loaded;

	Mutex					m_loaded_mutex;
	mutable Mutex			m_resources_mutex;

private:

	// Hashes of resource types (FIXME)
	uint32_t			m_texture_hash;
	uint32_t			m_mesh_hash;
	uint32_t			m_txt_hash;
	uint32_t			m_script_hash;

	friend class		Device;
};

} // namespace crown
