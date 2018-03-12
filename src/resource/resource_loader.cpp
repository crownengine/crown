/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/hash_map.h"
#include "core/containers/queue.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/path.h"
#include "core/memory/memory.h"
#include "core/memory/temp_allocator.h"
#include "core/os.h"
#include "core/strings/dynamic_string.h"
#include "device/log.h"
#include "resource/resource_loader.h"

namespace { const crown::log_internal::System RESOURCE_LOADER = { "ResourceLoader" }; }

namespace crown
{
static s32 thread_proc(void* thiz)
{
	return ((ResourceLoader*)thiz)->run();
}

ResourceLoader::ResourceLoader(Filesystem& data_filesystem)
	: _data_filesystem(data_filesystem)
	, _requests(default_allocator())
	, _loaded(default_allocator())
	, _fallback(default_allocator())
	, _exit(false)
{
	_thread.start(thread_proc, this);
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

u32 ResourceLoader::num_requests()
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

	const u32 num = queue::size(_loaded);
	array::reserve(loaded, num);

	for (u32 i = 0; i < num; ++i)
	{
		array::push_back(loaded, queue::front(_loaded));
		queue::pop_front(_loaded);
	}
}

void ResourceLoader::register_fallback(StringId64 type, StringId64 name)
{
	hash_map::set(_fallback, type, name);
}

s32 ResourceLoader::run()
{
	while (!_exit)
	{
		_mutex.lock();
		if (queue::empty(_requests))
		{
			_mutex.unlock();
			os::sleep(16);
			continue;
		}

		ResourceRequest rr = queue::front(_requests);
		_mutex.unlock();

		StringId64 mix;
		mix._id = rr.type._id ^ rr.name._id;

		TempAllocator128 ta;
		DynamicString res_path(ta);
		mix.to_string(res_path);

		DynamicString path(ta);
		path::join(path, CROWN_DATA_DIRECTORY, res_path.c_str());

		File* file = _data_filesystem.open(path.c_str(), FileOpenMode::READ);
		if (!file->is_open())
		{
			logw(RESOURCE_LOADER, "Can't load resource #ID(%s). Falling back...", res_path.c_str());

			StringId64 fallback_name;
			fallback_name = hash_map::get(_fallback, rr.type, fallback_name);
			CE_ENSURE(fallback_name._id != 0);

			mix._id = rr.type._id ^ fallback_name._id;
			mix.to_string(res_path);
			path::join(path, CROWN_DATA_DIRECTORY, res_path.c_str());

			_data_filesystem.close(*file);
			file = _data_filesystem.open(path.c_str(), FileOpenMode::READ);
		}
		CE_ASSERT(file->is_open(), "Can't load resource #ID(%s)", res_path.c_str());

		if (rr.load_function)
		{
			rr.data = rr.load_function(*file, *rr.allocator);
		}
		else
		{
			const u32 size = file->size();
			rr.data = rr.allocator->allocate(size);
			file->read(rr.data, size);
			CE_ASSERT(*(u32*)rr.data == rr.version, "Wrong version");
		}

		_data_filesystem.close(*file);

		add_loaded(rr);
		_mutex.lock();
		queue::pop_front(_requests);
		_mutex.unlock();
	}

	return 0;
}

} // namespace crown
