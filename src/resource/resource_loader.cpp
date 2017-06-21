/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/queue.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/path.h"
#include "core/memory/memory.h"
#include "core/memory/temp_allocator.h"
#include "core/os.h"
#include "core/strings/dynamic_string.h"
#include "resource/resource_loader.h"

namespace crown
{
ResourceLoader::ResourceLoader(Filesystem& data_filesystem)
	: _data_filesystem(data_filesystem)
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

bool ResourceLoader::can_load(StringId64 type, StringId64 name)
{
	TempAllocator128 ta;
	DynamicString type_str(ta);
	DynamicString name_str(ta);
	type.to_string(type_str);
	name.to_string(name_str);

	DynamicString res_path(ta);
	res_path += type_str;
	res_path += '-';
	res_path += name_str;

	DynamicString path(ta);
	path::join(path, CROWN_DATA_DIRECTORY, res_path.c_str());

	return _data_filesystem.exists(path.c_str());
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

		TempAllocator128 ta;
		DynamicString type_str(ta);
		DynamicString name_str(ta);
		rr.type.to_string(type_str);
		rr.name.to_string(name_str);

		DynamicString res_path(ta);
		res_path += type_str;
		res_path += '-';
		res_path += name_str;

		DynamicString path(ta);
		path::join(path, CROWN_DATA_DIRECTORY, res_path.c_str());

		File* file = _data_filesystem.open(path.c_str(), FileOpenMode::READ);

		if (rr.load_function)
		{
			rr.data = rr.load_function(*file, *rr.allocator);
		}
		else
		{
			const u32 size = file->size();
			void* data = rr.allocator->allocate(size);
			file->read(data, size);
			CE_ASSERT(*(u32*)data == rr.version, "Wrong version");
			rr.data = data;
		}

		_data_filesystem.close(*file);

		add_loaded(rr);
		_mutex.lock();
		queue::pop_front(_requests);
		_mutex.unlock();
	}

	return 0;
}

s32 ResourceLoader::thread_proc(void* thiz)
{
	return ((ResourceLoader*)thiz)->run();
}

} // namespace crown
