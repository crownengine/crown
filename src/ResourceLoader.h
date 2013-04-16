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

namespace crown
{

class ResourceManager;
class Allocator;
class ResourceArchive;

// Callbacks typedefs
typedef void (*ResourceLoadingCallback)(ResourceId);
typedef void (*ResourceOnlineCallback)(ResourceId, void*);

class ResourceLoader
{
public:

						ResourceLoader(Allocator& resource_allocator, ResourceArchive& archive);
						~ResourceLoader();

	void				load(ResourceId name);
	void				unload(ResourceId name, void* resource);

	void				flush();

	void				set_loading_callback(ResourceLoadingCallback f);
	void				set_online_callback(ResourceOnlineCallback f);

private:

	void*				load_by_type(ResourceId name);
	void				unload_by_type(ResourceId name, void* resource);

private:

	Allocator&			m_resource_allocator;
	ResourceArchive&	m_resource_archive;

	MallocAllocator		m_allocator;
	Queue<ResourceId>	m_resources;

	// Callbacks
	ResourceLoadingCallback m_loading_callback;
	ResourceOnlineCallback	m_online_callback;
	
	uint32_t			m_config_hash;
	uint32_t			m_texture_hash;
	uint32_t			m_mesh_hash;
	uint32_t			m_txt_hash;
};

} // namespace crown
