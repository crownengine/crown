/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "resource_manager.h"
#include "resource_registry.h"
#include "temp_allocator.h"
#include "sort_map.h"
#include "array.h"

namespace crown
{

const ResourceManager::ResourceEntry ResourceManager::ResourceEntry::NOT_FOUND = { 0xffffffffu, NULL };

ResourceManager::ResourceManager(ResourceLoader& rl)
	: _resource_heap("resource", default_allocator())
	, _loader(&rl)
	, _rm(default_allocator())
	, _autoload(false)
{
}

ResourceManager::~ResourceManager()
{
	const ResourceMap::Entry* begin = sort_map::begin(_rm);
	const ResourceMap::Entry* end = sort_map::end(_rm);

	for (; begin != end; begin++)
	{
		resource_on_offline(begin->pair.first.type, begin->pair.first.name, *this);
		resource_on_unload(begin->pair.first.type, _resource_heap, begin->pair.second.data);
	}
}

void ResourceManager::load(StringId64 type, StringId64 name)
{
	ResourcePair id = make_pair(type, name);
	ResourceEntry& entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);

	if (entry == ResourceEntry::NOT_FOUND)
	{
		_loader->add_request(type, name, _resource_heap);
		return;
	}

	entry.references++;
}

void ResourceManager::unload(StringId64 type, StringId64 name)
{
	flush();

	ResourcePair id = make_pair(type, name);
	ResourceEntry& entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);

	if (--entry.references == 0)
	{
		resource_on_offline(type, name, *this);
		resource_on_unload(type, _resource_heap, entry.data);

		sort_map::remove(_rm, id);
		sort_map::sort(_rm);
	}
}

void ResourceManager::reload(StringId64 type, StringId64 name)
{
	const ResourcePair id = make_pair(type, name);
	const ResourceEntry& entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);
	const uint32_t old_refs = entry.references;

	unload(type, name);
	load(type, name);
	flush();

	ResourceEntry& new_entry = sort_map::get(_rm, id, ResourceEntry::NOT_FOUND);
	new_entry.references = old_refs;
}

bool ResourceManager::can_get(StringId64 type, StringId64 name)
{
	return _autoload ? true : sort_map::has(_rm, make_pair(type, name));
}

const void* ResourceManager::get(StringId64 type, StringId64 name)
{
	const ResourcePair id = make_pair(type, name);
	char type_buf[StringId64::STRING_LENGTH];
	char name_buf[StringId64::STRING_LENGTH];

	CE_ASSERT(can_get(type, name), "Resource not loaded #ID(%s-%s)", type.to_string(type_buf), name.to_string(name_buf));
	CE_UNUSED(type_buf);
	CE_UNUSED(name_buf);

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
	Array<ResourceData> loaded(ta);
	_loader->get_loaded(loaded);

	for (uint32_t i = 0; i < array::size(loaded); i++)
		complete_request(loaded[i].type, loaded[i].name, loaded[i].data);
}

void ResourceManager::complete_request(StringId64 type, StringId64 name, void* data)
{
	ResourceEntry entry;
	entry.references = 1;
	entry.data = data;

	sort_map::set(_rm, make_pair(type, name), entry);
	sort_map::sort(_rm);

	resource_on_online(type, name, *this);
}

} // namespace crown
