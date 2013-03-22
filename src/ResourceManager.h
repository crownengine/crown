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

struct ResourceEntry
{
	ResourceId		id;
	ResourceState	state;

	uint32_t		references;

	void*			resource;

	bool			operator==(const StringId32& name)
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

							ResourceManager();
							~ResourceManager();

	ResourceId				load(const char* name);
	ResourceId				load(StringId32 name);

	void					unload(ResourceId name);
	void					reload(ResourceId name);

	bool					has(ResourceId name);
	bool					is_loaded(ResourceId name);
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

