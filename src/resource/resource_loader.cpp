/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "resource_loader.h"
#include "config.h"
#include "memory.h"
#include "queue.h"
#include "filesystem.h"
#include "temp_allocator.h"
#include "path.h"

namespace crown
{

ResourceLoader::ResourceLoader(Filesystem& fs)
	: _fs(fs)
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

void ResourceLoader::add_request(const ResourceRequest& rr)
{
	ScopedMutex sm(_mutex);
	queue::push_back(_requests, rr);
}

void ResourceLoader::flush()
{
	while (num_requests()) {}
}

uint32_t ResourceLoader::num_requests()
{
	ScopedMutex sm(_mutex);
	return queue::size(_requests);
}

void ResourceLoader::add_loaded(ResourceRequest rr)
{
	ScopedMutex sm(_loaded_mutex);
	queue::push_back(_loaded, rr);
}

void ResourceLoader::get_loaded(Array<ResourceRequest>& loaded)
{
	ScopedMutex sm(_loaded_mutex);

	const uint32_t num = queue::size(_loaded);
	array::reserve(loaded, num);

	for (uint32_t i = 0; i < num; ++i)
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
		ResourceRequest rr = queue::front(_requests);
		_mutex.unlock();

		char name[1 + 2*StringId64::STRING_LENGTH];
		rr.type.to_string(name);
		name[16] = '-';
		rr.name.to_string(name + 17);

		TempAllocator256 alloc;
		DynamicString path(alloc);
		path::join(CROWN_DATA_DIRECTORY, name, path);

		File* file = _fs.open(path.c_str(), FileOpenMode::READ);
		rr.data = rr.load_function(*file, *rr.allocator);
		_fs.close(*file);

		add_loaded(rr);
		_mutex.lock();
		queue::pop_front(_requests);
		_mutex.unlock();
	}

	return 0;
}

} // namespace crown
