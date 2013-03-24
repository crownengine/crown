#include "ResourceLoader.h"
#include "ResourceManager.h"
#include "String.h"
#include "TextureResource.h"
#include <stdio.h>
#include <unistd.h>

namespace crown
{

ResourceLoader::ResourceLoader(ResourceManager* resource_manager) :
	m_resource_manager(resource_manager),
	m_resources(m_allocator)
{
	m_config_hash = string::Hash32("config");
	m_texture_hash = string::Hash32("tga");
	m_mesh_hash = string::Hash32("mesh");
}

ResourceLoader::~ResourceLoader()
{
}

void ResourceLoader::load(ResourceId name)
{
	m_resources.push_back(name);
	
	// callback to the resource manager
	m_resource_manager->loading(name);
}

void ResourceLoader::unload(ResourceId name)
{
	// do something
}

void ResourceLoader::flush()
{
	while (m_resources.size() > 0)
	{
		ResourceId& resource = m_resources.front();
		
		void* data = load_by_type(resource);

		m_resource_manager->online(m_resources.front(), data);

		m_resources.pop_front();
	}
}

void* ResourceLoader::load_by_type(ResourceId name)
{
	if (name.name == m_config_hash)
	{
		return NULL;
	}

	if (name.name == m_texture_hash)
	{
		return TextureResource::load(NULL);
	}
}

} // namespace crown

