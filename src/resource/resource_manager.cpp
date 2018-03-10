/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.h"
#include "core/containers/sort_map.h"
#include "core/memory/temp_allocator.h"
#include "core/strings/dynamic_string.h"
#include "resource/resource_loader.h"
#include "resource/resource_manager.h"

namespace crown
{
const ResourceManager::ResourceEntry ResourceManager::ResourceEntry::NOT_FOUND = { 0xffffffffu, NULL };

ResourceManager::ResourceManager(ResourceLoader& rl)
	: _resource_heap(default_allocator(), "resource")
	, _loader(&rl)
	, _type_data(default_allocator())
	, _rm(default_allocator())
	, _autoload(false)
{
}

ResourceManager::~ResourceManager()
{
	auto cur = sort_map::begin(_rm);
	auto end = sort_map::end(_rm);
	for (; cur != end; ++cur)
	{
		const StringId64 type = cur->first.type;
		const StringId64 name = cur->first.name;
		on_offline(type, name);
		on_unload(type, cur->second.data);
	}
}

void ResourceManager::load(StringId64 type, StringId64 name)
{
	ResourcePair id = { type, name };
	ResourceEntry& entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);

	if (entry == ResourceEntry::NOT_FOUND)
	{
		ResourceTypeData rtd;
		rtd.version = UINT32_MAX;
		rtd.load = NULL;
		rtd.online = NULL;
		rtd.offline = NULL;
		rtd.unload = NULL;
		rtd = sort_map::get(_type_data, type, rtd);

		ResourceRequest rr;
		rr.type = type;
		rr.name = name;
		rr.version = rtd.version;
		rr.load_function = rtd.load;
		rr.allocator = &_resource_heap;
		rr.data = NULL;

		_loader->add_request(rr);
		return;
	}

	entry.references++;
}

void ResourceManager::unload(StringId64 type, StringId64 name)
{
	flush();

	ResourcePair id = { type, name };
	ResourceEntry& entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);

	if (--entry.references == 0)
	{
		on_offline(type, name);
		on_unload(type, entry.data);

		sort_map::remove(_rm, id);
		sort_map::sort(_rm);
	}
}

void ResourceManager::reload(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	const ResourceEntry& entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);
	const u32 old_refs = entry.references;

	unload(type, name);
	load(type, name);
	flush();

	ResourceEntry& new_entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);
	new_entry.references = old_refs;
}

bool ResourceManager::can_get(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	return _autoload ? true : sort_map::has(_rm, id);
}

const void* ResourceManager::get(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };

	StringId64 mix;
	mix._id = type._id ^ name._id;

	TempAllocator64 ta;
	DynamicString path(ta);
	mix.to_string(path);

	CE_ASSERT(can_get(type, name), "Resource not loaded #ID(%s)", path.c_str());

	if (_autoload && !sort_map::has(_rm, id))
	{
		load(type, name);
		flush();
	}

	const ResourceEntry& entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);
	return entry.data;
}

void ResourceManager::enable_autoload(bool enable)
{
	_autoload = enable;
}

void ResourceManager::flush()
{
	_loader->flush();
	complete_requests();
}

void ResourceManager::complete_requests()
{
	TempAllocator1024 ta;
	Array<ResourceRequest> loaded(ta);
	_loader->get_loaded(loaded);

	for (u32 i = 0; i < array::size(loaded); ++i)
		complete_request(loaded[i].type, loaded[i].name, loaded[i].data);
}

void ResourceManager::complete_request(StringId64 type, StringId64 name, void* data)
{
	ResourceEntry entry;
	entry.references = 1;
	entry.data = data;

	ResourcePair id = { type, name };

	sort_map::set(_rm, id, entry);
	sort_map::sort(_rm);

	on_online(type, name);
}

void ResourceManager::register_type(StringId64 type, u32 version, LoadFunction load, UnloadFunction unload, OnlineFunction online, OfflineFunction offline)
{
	ResourceTypeData rtd;
	rtd.version = version;
	rtd.load = load;
	rtd.online = online;
	rtd.offline = offline;
	rtd.unload = unload;

	sort_map::set(_type_data, type, rtd);
	sort_map::sort(_type_data);
}

void ResourceManager::on_online(StringId64 type, StringId64 name)
{
	OnlineFunction func = sort_map::get(_type_data, type, ResourceTypeData()).online;

	if (func)
		func(name, *this);
}

void ResourceManager::on_offline(StringId64 type, StringId64 name)
{
	OfflineFunction func = sort_map::get(_type_data, type, ResourceTypeData()).offline;

	if (func)
		func(name, *this);
}

void ResourceManager::on_unload(StringId64 type, void* data)
{
	UnloadFunction func = sort_map::get(_type_data, type, ResourceTypeData()).unload;

	if (func)
		func(_resource_heap, data);
	else
		_resource_heap.deallocate(data);
}

} // namespace crown
