#pragma once

#include "Queue.h"
#include "Resource.h"
#include "MallocAllocator.h"
#include "ResourceArchive.h"

namespace crown
{

class ResourceManager;

class ResourceLoader
{
public:

						ResourceLoader(ResourceManager* resource_manager);
						~ResourceLoader();

	void				load(ResourceId name);
	void				unload(ResourceId name);

	void				flush();

private:

	void*				load_by_type(ResourceId name);

private:

	ResourceManager*	m_resource_manager;
	ResourceArchive		m_resource_archive;
	MallocAllocator		m_allocator;
	Queue<ResourceId>	m_resources;
	
	StringId32			m_config_hash;
	StringId32			m_texture_hash;
	StringId32			m_mesh_hash;
};

} // namespace crown

