/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "resource_loader.h"
#include "memory.h"
#include "resource_registry.h"
#include "queue.h"
#include "filesystem.h"
#include "temp_allocator.h"
#include "path.h"

namespace crown
{

ResourceLoader::ResourceLoader(Filesystem& fs, Allocator& resource_heap)
	: _fs(fs)
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

		char name[64];
		id.to_string(name);

		TempAllocator256 alloc;
		DynamicString path(alloc);
		path::join(CROWN_DATA_DIRECTORY, name, path);

		File* file = _fs.open(path.c_str(), FOM_READ);
		rd.data = resource_on_load(id.type, *file, _resource_heap);
		_fs.close(file);

		add_loaded(rd);
		_mutex.lock();
		queue::pop_front(_requests);
		_mutex.unlock();
	}

	return 0;
}

} // namespace crown
