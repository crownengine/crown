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

#include "Queue.h"
#include "Resource.h"
#include "MallocAllocator.h"
#include "ResourceManager.h"
#include "OS.h"
#include "Thread.h"
#include "Mutex.h"

namespace crown
{

class Allocator;
class ResourceArchive;

struct LoadedResource
{
	ResourceId	resource;
	void*		data;
};

class ResourceLoader
{
public:

						ResourceLoader(Allocator& resource_allocator, ResourceArchive& archive);
						~ResourceLoader();

	void				load(ResourceId name);
	void				unload(ResourceId name, void* resource);

private:

	void				background_load();
	void*				load_by_type(ResourceId name) const;
	void				unload_by_type(ResourceId name, void* resource) const;

private:

	static void*		background_thread(void* thiz);

private:

	Allocator&			m_resource_allocator;
	ResourceArchive&	m_resource_archive;

	MallocAllocator		m_allocator;

	Mutex				m_waiting_mutex;
	Queue<ResourceId>	m_waiting_resources;
	Mutex				m_loaded_mutex;
	Queue<LoadedResource>	m_loaded_resources;

	Thread				m_thread;

	uint32_t			m_config_hash;
	uint32_t			m_texture_hash;
	uint32_t			m_mesh_hash;
	uint32_t			m_txt_hash;

	friend class		ResourceManager;
};

} // namespace crown
