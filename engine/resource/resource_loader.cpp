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

#include "resource_loader.h"
#include "memory.h"
#include "resource_registry.h"
#include "log.h"
#include "queue.h"
#include "bundle.h"

namespace crown
{

ResourceLoader::ResourceLoader(Bundle& bundle, Allocator& resource_heap)
	: m_thread()
	, m_bundle(bundle)
	, m_resource_heap(resource_heap)
	, m_requests(default_allocator())
	, m_loaded(default_allocator())
	, m_exit(false)
{
	m_thread.start(ResourceLoader::thread_proc, this);
}

ResourceLoader::~ResourceLoader()
{
	m_exit = true;
	m_thread.stop();
}

void ResourceLoader::load(ResourceId id)
{
	add_request(id);
}

void ResourceLoader::flush()
{
	while (num_requests()) {}
}

void ResourceLoader::add_request(ResourceId id)
{
	ScopedMutex sm(m_mutex);
	queue::push_back(m_requests, id);
}

uint32_t ResourceLoader::num_requests()
{
	ScopedMutex sm(m_mutex);
	return queue::size(m_requests);
}

void ResourceLoader::add_loaded(ResourceData data)
{
	ScopedMutex sm(m_loaded_mutex);
	queue::push_back(m_loaded, data);
}

void ResourceLoader::get_loaded(Array<ResourceData>& loaded)
{
	ScopedMutex sm(m_loaded_mutex);
	uint32_t num = queue::size(m_loaded);
	for (uint32_t i = 0; i < num; i++)
	{
		array::push_back(loaded, queue::front(m_loaded));
		queue::pop_front(m_loaded);
	}
}

int32_t ResourceLoader::run()
{
	while (!m_exit)
	{
		m_mutex.lock();
		if (queue::empty(m_requests))
		{
			m_mutex.unlock();
			continue;
		}
		ResourceId id = queue::front(m_requests);
		m_mutex.unlock();

		ResourceData rd;
		rd.id = id;
		File* file = m_bundle.open(id);
		rd.data = resource_on_load(id.type, *file, m_resource_heap);
		m_bundle.close(file);
		add_loaded(rd);
		m_mutex.lock();
		queue::pop_front(m_requests);
		m_mutex.unlock();
	}

	return 0;
}

} // namespace crown
