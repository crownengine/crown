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
	: _bundle(bundle)
	, _resource_heap(resource_heap)
	, _requests(default_allocator())
	, _loaded(default_allocator())
	, _exit(false)
{
	_thread.start(ResourceLoader::thread_proc, this);
}

ResourceLoader::~ResourceLoader()
{
	_exit = true;
	_thread.stop();
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
	ScopedMutex sm(_mutex);
	queue::push_back(_requests, id);
}

uint32_t ResourceLoader::num_requests()
{
	ScopedMutex sm(_mutex);
	return queue::size(_requests);
}

void ResourceLoader::add_loaded(ResourceData data)
{
	ScopedMutex sm(_loaded_mutex);
	queue::push_back(_loaded, data);
}

void ResourceLoader::get_loaded(Array<ResourceData>& loaded)
{
	ScopedMutex sm(_loaded_mutex);
	uint32_t num = queue::size(_loaded);
	for (uint32_t i = 0; i < num; i++)
	{
		array::push_back(loaded, queue::front(_loaded));
		queue::pop_front(_loaded);
	}
}

int32_t ResourceLoader::run()
{
	while (!_exit)
	{
		_mutex.lock();
		if (queue::empty(_requests))
		{
			_mutex.unlock();
			continue;
		}
		ResourceId id = queue::front(_requests);
		_mutex.unlock();

		ResourceData rd;
		rd.id = id;
		File* file = _bundle.open(id);
		rd.data = resource_on_load(id.type, *file, _resource_heap);
		_bundle.close(file);
		add_loaded(rd);
		_mutex.lock();
		queue::pop_front(_requests);
		_mutex.unlock();
	}

	return 0;
}

} // namespace crown
