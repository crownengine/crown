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

#include "ResourceLoader.h"
#include "TextureResource.h"
#include "MeshResource.h"
#include "SoundResource.h"

namespace crown
{

//-----------------------------------------------------------------------------
ResourceLoader::ResourceLoader(Bundle& bundle) :
	Thread("resource-loader"),
	m_bundle(bundle),
	m_load_queue(m_allocator),
	m_done_queue(m_allocator)
{
}

//-----------------------------------------------------------------------------
void ResourceLoader::load(ResourceId resource)
{
	m_load_mutex.lock();
	m_load_queue.push_back(resource);
	m_load_requests.signal();
	m_load_mutex.unlock();
}

//-----------------------------------------------------------------------------
void ResourceLoader::unload(ResourceId resource, void* data)
{
	unload_by_type(resource, data);
}

//-----------------------------------------------------------------------------
uint32_t ResourceLoader::remaining() const
{
	return m_load_queue.size();
}

//-----------------------------------------------------------------------------
uint32_t ResourceLoader::num_loaded() const
{
	return m_done_queue.size();
}

//-----------------------------------------------------------------------------
void ResourceLoader::get_loaded(List<LoadedResource>& l)
{
	m_done_mutex.lock();
	for (uint32_t i = 0; i < m_done_queue.size(); i++)
	{
		l.push_back(m_done_queue[i]);
	}

	m_done_queue.clear();
	m_done_mutex.unlock();
}

//-----------------------------------------------------------------------------
int32_t ResourceLoader::run()
{
	while (!is_terminating())
	{
		m_load_mutex.lock();
		while (m_load_queue.size() == 0)
		{
			m_load_requests.wait(m_load_mutex);
		}

		ResourceId resource = m_load_queue.front();
		m_load_queue.pop_front();
		m_load_mutex.unlock();

		void* data = load_by_type(resource);

		m_done_mutex.lock();
		m_done_queue.push_back(LoadedResource(resource, data));
		m_done_mutex.unlock();
	}

	return 0;
}

//-----------------------------------------------------------------------------
void* ResourceLoader::load_by_type(ResourceId name)
{
	switch (name.type)
	{
		case TEXTURE_TYPE: return TextureResource::load(m_resource_allocator, m_bundle, name);
		case MESH_TYPE: return MeshResource::load(m_resource_allocator, m_bundle, name);
		case SOUND_TYPE: return SoundResource::load(m_resource_allocator, m_bundle, name);
		default: return NULL;
	}
}

//-----------------------------------------------------------------------------
void ResourceLoader::unload_by_type(ResourceId name, void* data)
{
	switch (name.type)
	{
		case TEXTURE_TYPE: return TextureResource::unload(m_resource_allocator, data);
		case MESH_TYPE: return MeshResource::unload(m_resource_allocator, data);
		case SOUND_TYPE: return SoundResource::unload(m_resource_allocator, data);
	}
}

} // namespace crown
