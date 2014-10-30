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

#include <algorithm>
#include <inttypes.h>
#include "types.h"
#include "resource_manager.h"
#include "resource_registry.h"
#include "string_utils.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "dynamic_string.h"
#include "queue.h"
#include "sort_map.h"

namespace crown
{

ResourceId::ResourceId(const char* type, const char* name)
	: type(string::murmur2_64(type, string::strlen(type), 0))
	, name(string::murmur2_64(name, string::strlen(name), 0))
{
}

ResourceManager::ResourceManager(Bundle& bundle)
	: _resource_heap("resource", default_allocator())
	, _loader(bundle, _resource_heap)
	, _rm(default_allocator())
	, _autoload(false)
{
}

const ResourceManager::ResourceEntry ResourceManager::NOT_FOUND = { 0xffffffffu, NULL };

ResourceManager::~ResourceManager()
{
	const ResourceMap::Entry* begin = sort_map::begin(_rm);
	const ResourceMap::Entry* end = sort_map::end(_rm);

	for (; begin != end; begin++)
	{
		resource_on_offline(begin->key.type, begin->key.name, *this);
		resource_on_unload(begin->key.type, _resource_heap, begin->value.data);
	}
}

void ResourceManager::load(StringId64 type, StringId64 name)
{
	ResourceId id(type, name);
	ResourceEntry& entry = sort_map::get(_rm, id, NOT_FOUND);

	if (entry == NOT_FOUND)
	{
		_loader.load(id);
		return;
	}

	entry.references++;
}

void ResourceManager::unload(StringId64 type, StringId64 name)
{
	flush();

	ResourceId id(type, name);
	ResourceEntry& entry = sort_map::get(_rm, id, NOT_FOUND);

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
	const ResourceId id(type, name);
	const ResourceEntry& entry = sort_map::get(_rm, id, NOT_FOUND);
	const uint32_t old_refs = entry.references;

	unload(type, name);
	load(type, name);
	flush();

	ResourceEntry& new_entry = sort_map::get(_rm, id, NOT_FOUND);
	new_entry.references = old_refs;
}

bool ResourceManager::can_get(const char* type, const char* name)
{
	ResourceId id(type, name);
	return can_get(id.type, id.name);
}

bool ResourceManager::can_get(StringId64 type, StringId64 name)
{
	return _autoload ? true : sort_map::has(_rm, ResourceId(type, name));
}

const void* ResourceManager::get(const char* type, const char* name)
{
	ResourceId id(type, name);
	return get(id.type, id.name);
}

const void* ResourceManager::get(StringId64 type, StringId64 name)
{
	ResourceId id(type, name);

	if (_autoload && !sort_map::has(_rm, id))
	{
		load(type, name);
		flush();
	}

	const ResourceEntry& entry = sort_map::get(_rm, id, NOT_FOUND);
	return entry.data;
}

void ResourceManager::enable_autoload(bool enable)
{
	_autoload = enable;
}

void ResourceManager::flush()
{
	_loader.flush();
	complete_requests();
}

void ResourceManager::complete_requests()
{
	TempAllocator1024 ta;
	Array<ResourceData> loaded(ta);
	_loader.get_loaded(loaded);

	for (uint32_t i = 0; i < array::size(loaded); i++)
		complete_request(loaded[i].id, loaded[i].data);
}

void ResourceManager::complete_request(ResourceId id, void* data)
{
	ResourceEntry entry;
	entry.references = 1;
	entry.data = data;

	sort_map::set(_rm, id, entry);
	sort_map::sort(_rm);

	resource_on_online(id.type, id.name, *this);
}

} // namespace crown
