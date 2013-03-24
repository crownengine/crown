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
	
	uint32_t			m_config_hash;
	uint32_t			m_texture_hash;
	uint32_t			m_mesh_hash;
};

} // namespace crown

