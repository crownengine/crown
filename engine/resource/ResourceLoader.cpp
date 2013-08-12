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
#include "ResourceRegistry.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
ResourceLoader::ResourceLoader(Bundle& bundle, Allocator& resource_heap) :
	Thread("resource-loader"),
	m_bundle(bundle),
	m_resource_heap(resource_heap),
	m_load_queue(default_allocator()),
	m_done_queue(default_allocator())
{
}

//-----------------------------------------------------------------------------
void ResourceLoader::load(ResourceId resource)
{
	m_load_mutex.lock();
	m_load_queue.push_back(resource);
	Log::i("Signaling...");
	m_load_requests.signal();
	m_load_mutex.unlock();
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
			Log::i("Waiting...");
			m_load_requests.wait(m_load_mutex);
		}
		Log::i("Loading...");
		ResourceId resource = m_load_queue.front();
		m_load_queue.pop_front();
		m_load_mutex.unlock();

		void* data = resource_on_load(resource.type, m_resource_heap, m_bundle, resource);

		m_done_mutex.lock();
		m_done_queue.push_back(LoadedResource(resource, data));
		m_done_mutex.unlock();
	}

	return 0;
}

} // namespace crown
