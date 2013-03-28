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
#include "Resource.h"
#include "ResourceLoader.h"
#include "MallocAllocator.h"

namespace crown
{

class Filesystem;

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

class ResourceLoader;

/// Resource manager.
class ResourceManager
{
public:

							ResourceManager(Filesystem* filesystem);
							~ResourceManager();

	/// Loads the resource by @name and returns its ResourceId.
	/// Note that the resource data may be not immediately available,
	/// the resource gets pushed in a queue of load requests and loadead as
	/// soon as possible by the ResourceLoader.
	/// You have to explicitly call is_loaded() method to check if the
	/// loading process is actually completed.
	ResourceId				load(const char* name);
	
	/// Loads the resource by @name and @type and returns its ResourceId.
	/// See ResourceManager::load(const char* name) for details.
	ResourceId				load(uint32_t name, uint32_t type);

	void					unload(ResourceId name);

	void					reload(ResourceId name);

	/// Returns whether the manager has the @name resource into
	/// its list of resources.
	/// Note that having a resource does not mean that the resource is
	/// available for using; instead, you have to check is_loaded() to
	/// obtain the resource availability status.
	bool					has(ResourceId name);

	/// Returns the data associated with the @name resource.
	/// The resource data contains resource-specific metadata
	/// and the actual resource data. In order to correctly use
	/// it, you have to know which type of data @name refers to
	/// and cast accordingly.
	void*					data(ResourceId name);
	
	/// Returns whether the @name resource is loaded (i.e. whether
	/// you can use the data associated with it).
	bool					is_loaded(ResourceId name);
	
	/// Forces the loading of all of the queued resource load requests.
	void					flush() { m_resource_loader.flush(); }

private:

	void					loading(ResourceId name);
	void					online(ResourceId name, void* resource);

private:

	ResourceLoader			m_resource_loader;
	MallocAllocator			m_allocator;
	List<ResourceEntry>		m_resources;
	
	friend class			ResourceLoader;
};

} // namespace crown

