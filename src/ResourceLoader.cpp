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
#include "ResourceArchive.h"
#include "String.h"
#include "Hash.h"
#include "TextureResource.h"
#include "TextResource.h"
#include <stdio.h>
#include <unistd.h>

namespace crown
{

//-----------------------------------------------------------------------------
ResourceLoader::ResourceLoader(Allocator& resource_allocator, ResourceArchive& archive) :
	m_resource_allocator(resource_allocator),
	m_resource_archive(archive),
	m_resources(m_allocator),
	m_loading_callback(NULL),
	m_online_callback(NULL)
{
	// FIXME hardcoded seed
	m_config_hash = hash::murmur2_32("config", string::strlen("config"), 0);
	m_texture_hash = hash::murmur2_32("tga", string::strlen("tga"), 0);
	m_mesh_hash = hash::murmur2_32("mesh", string::strlen("mesh"), 0);
	m_txt_hash = hash::murmur2_32("txt", 3, 0);
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
	if (m_loading_callback != NULL)
	{
		m_loading_callback(name);
	}
}

//-----------------------------------------------------------------------------
void ResourceLoader::unload(ResourceId name, void* resource)
{
	unload_by_type(name, resource);
}

//-----------------------------------------------------------------------------
void ResourceLoader::flush()
{
	while (m_resources.size() > 0)
	{
		ResourceId& resource = m_resources.front();
		
		void* data = load_by_type(resource);

		if (m_online_callback != NULL)
		{
			m_online_callback(m_resources.front(), data);
		}

		m_resources.pop_front();
	}
}

//-----------------------------------------------------------------------------
void ResourceLoader::set_loading_callback(ResourceLoadingCallback f)
{
	m_loading_callback = f;
}

//-----------------------------------------------------------------------------
void ResourceLoader::set_online_callback(ResourceOnlineCallback f)
{
	m_online_callback = f;
}

//-----------------------------------------------------------------------------
void* ResourceLoader::load_by_type(ResourceId name)
{
	if (name.type == m_config_hash)
	{
		return NULL;
	}

	if (name.type == m_texture_hash)
	{
		return TextureResource::load(m_resource_allocator, &m_resource_archive, name);
	}

	if (name.type == m_txt_hash)
	{
		return TextResource::load(m_resource_allocator, &m_resource_archive, name);
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void ResourceLoader::unload_by_type(ResourceId name, void* resource)
{
	if (name.type == m_config_hash)
	{
		return;
	}

	if (name.type == m_texture_hash)
	{
		TextureResource::unload(m_resource_allocator, (TextureResource*)resource);
	}

	if (name.type == m_txt_hash)
	{
		TextResource::unload(m_resource_allocator, (TextResource*)resource);
	}

	return;
}

} // namespace crown
