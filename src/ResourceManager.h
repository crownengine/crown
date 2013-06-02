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

	bool			operator==(const ResourceId& resource)
					{
						return id == resource;
					}

	bool			operator==(const ResourceEntry& b)
					{
						return id == b.id;
					}
};

struct LoadedResource
{
	ResourceId	resource;
	void*		data;
};

class Bundle;

/// Resource manager.
class ResourceManager
{
public:

	/// Read resources from @a bundle and store resource data using @a allocator.
							ResourceManager(Bundle& bundle);
							~ResourceManager();

	/// Loads the resource by @a name and returns its ResourceId.
	/// @note
	/// The resource data may be not immediately available,
	/// the resource gets pushed in a queue of load requests and loadead as
	/// soon as possible by the ResourceLoader.
	/// You have to explicitly call is_loaded() method to check if the
	/// loading process is actually completed.
	ResourceId				load(const char* name);

	/// Unloads the @a resource, freeing up all the memory associated by it
	/// and eventually any global object associated with it.
	/// (Such as texture objects, vertex buffers etc.)
	void					unload(ResourceId name);

	/// Reloads the @a resource
	void					reload(ResourceId name);

	/// Returns whether the manager has the @a name resource into
	/// its list of resources.
	/// @note
	/// Having a resource does not mean that the resource is
	/// available for using; instead, you have to check is_loaded() to
	/// obtain the resource availability status.
	bool					has(ResourceId name) const;

	/// Returns the data associated with the @a name resource.
	/// The resource data contains resource-specific metadata
	/// and the actual resource data. In order to correctly use
	/// it, you have to know which type of data @a name refers to
	/// and cast accordingly.
	const void*				data(ResourceId name) const;
	
	/// Returns whether the @a name resource is loaded (i.e. whether
	/// you can use the data associated with it).
	bool					is_loaded(ResourceId name) const;

	/// Returns the number of references to the @a resource
	uint32_t				references(ResourceId name) const;

	/// Returns the number of resources still waiting to load.
	uint32_t				remaining() const;

	/// Forces all the loading requests to complete before preceeding.
	void					flush();

	/// Returns the seed used to generate resource name hashes.
	uint32_t				seed() const;

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

	void*					load_by_type(ResourceId name);
	void					unload_by_type(ResourceId name, void* resource);
	void					online(ResourceId name, void* resource);

private:

	static void*			background_thread(void* thiz);

private:

	// Archive whether to look for resources
	Bundle&					m_resource_bundle;
	// Used to strore resource memory
	MallocAllocator			m_resource_allocator;

	MallocAllocator			m_allocator;
	// The master lookup table
	List<ResourceEntry>		m_resources;

	// Resources waiting for loading
	Queue<ResourceId>		m_loading_queue;
	// Resources already loaded, ready to bring online
	Queue<LoadedResource>	m_loaded_queue;

	uint32_t				m_seed;

	// Background loading thread
	bool 					m_background_thread_should_run;
	os::Thread				m_thread;

	mutable os::Mutex		m_loading_mutex;
	os::Cond 				m_loading_requests;
	os::Cond 				m_all_loaded;

	os::Mutex				m_loaded_mutex;
	mutable os::Mutex		m_resources_mutex;

	friend class			Device;
};

} // namespace crown
