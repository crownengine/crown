/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/hash_map.inl"
#include "core/containers/queue.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/path.h"
#include "core/memory/globals.h"
#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "core/thread/scoped_mutex.inl"
#include "device/log.h"
#include "resource/resource_id.inl"
#include "resource/resource_loader.h"
#include "resource/types.h"

LOG_SYSTEM(RESOURCE_LOADER, "resource_loader")

namespace crown
{
ResourceLoader::ResourceLoader(Filesystem& data_filesystem)
	: _data_filesystem(data_filesystem)
	, _requests(default_allocator())
	, _loaded(default_allocator())
	, _fallback(default_allocator())
	, _exit(false)
{
	_thread.start([](void* thiz) { return ((ResourceLoader*)thiz)->run(); }, this);
}

ResourceLoader::~ResourceLoader()
{
	_exit = true;
	_requests_condition.signal(); // Spurious wake to exit thread
	_thread.stop();
}

void ResourceLoader::add_request(const ResourceRequest& rr)
{
	ScopedMutex sm(_mutex);
	queue::push_back(_requests, rr);
	_requests_condition.signal();
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
	while (1)
	{
		_mutex.lock();
		while (queue::empty(_requests) && !_exit)
			_requests_condition.wait(_mutex);

		if (_exit)
			break;

		ResourceRequest rr = queue::front(_requests);
		_mutex.unlock();

		ResourceId res_id = resource_id(rr.type, rr.name);

		TempAllocator128 ta;
		DynamicString path(ta);
		destination_path(path, res_id);

		File* file = _data_filesystem.open(path.c_str(), FileOpenMode::READ);
		if (!file->is_open())
		{
			logw(RESOURCE_LOADER, "Can't load resource: " RESOURCE_ID_FMT ". Falling back...", res_id._id);

			StringId64 fallback_name;
			fallback_name = hash_map::get(_fallback, rr.type, fallback_name);
			CE_ENSURE(fallback_name._id != 0);

			res_id = resource_id(rr.type, fallback_name);
			destination_path(path, res_id);

			_data_filesystem.close(*file);
			file = _data_filesystem.open(path.c_str(), FileOpenMode::READ);
		}
		CE_ASSERT(file->is_open(), "Can't load fallback resource: " RESOURCE_ID_FMT, res_id._id);

		if (rr.load_function)
		{
			rr.data = rr.load_function(*file, *rr.allocator);
		}
		else
		{
			const u32 size = file->size();
			rr.data = rr.allocator->allocate(size, 16);
			file->read(rr.data, size);
			CE_ASSERT(*(u32*)rr.data == RESOURCE_HEADER(rr.version), "Wrong version");
		}

		_data_filesystem.close(*file);

		add_loaded(rr);
		_mutex.lock();
		queue::pop_front(_requests);
		_mutex.unlock();
	}

	_mutex.unlock();
	return 0;
}

} // namespace crown
