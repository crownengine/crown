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

#include "ResourceLoader.h"
#include "ResourceManager.h"
#include "String.h"
#include "Hash.h"
#include "TextureResource.h"
#include <stdio.h>
#include <unistd.h>

namespace crown
{

//-----------------------------------------------------------------------------
ResourceLoader::ResourceLoader(ResourceManager* resource_manager, Filesystem* filesystem) :
	m_resource_manager(resource_manager),
	m_filesystem(filesystem),
	m_resources(m_allocator)
{
	m_config_hash = hash::fnv1a_32("config", string::strlen("config"));
	m_texture_hash = hash::fnv1a_32("tga", string::strlen("tga"));
	m_mesh_hash = hash::fnv1a_32("mesh", string::strlen("mesh"));
}

//-----------------------------------------------------------------------------
ResourceLoader::~ResourceLoader()
{
}

//-----------------------------------------------------------------------------
void ResourceLoader::load(ResourceId name)
{
	m_resources.push_back(name);
	
	// callback to the resource manager
	m_resource_manager->loading(name);
}

//-----------------------------------------------------------------------------
void ResourceLoader::unload(ResourceId name)
{
	// do something
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void* ResourceLoader::load_by_type(ResourceId name)
{
	if (name.name == m_config_hash)
	{
		return NULL;
	}

	if (name.name == m_texture_hash)
	{
		return TextureResource::load(&m_resource_archive, name.name);
	}
	
	return NULL;
}

} // namespace crown

