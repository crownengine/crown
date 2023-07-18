/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "resource/resource_id.inl"
#include "resource/resource_loader.h"
#include "resource/resource_manager.h"

namespace crown
{
bool operator<(const ResourceManager::ResourcePair &a, const ResourceManager::ResourcePair &b)
{
	return a.type < b.type
		|| (a.type == b.type && a.name < b.name)
		;
}

bool operator==(const ResourceManager::ResourcePair &a, const ResourceManager::ResourcePair &b)
{
	return a.type == b.type
		&& a.name == b.name
		;
}

bool operator==(const ResourceManager::ResourceEntry &a, const ResourceManager::ResourceEntry &b)
{
	return a.references == b.references
		&& a.data == b.data
		;
}

const ResourceManager::ResourceEntry ResourceManager::ResourceEntry::NOT_FOUND = { 0xffffffffu, NULL, NULL };

template<>
struct hash<ResourceManager::ResourcePair>
{
	u32 operator()(const ResourceManager::ResourcePair &val) const
	{
		return u32(resource_id(val.type, val.name)._id);
	}
};

ResourceManager::ResourceManager(ResourceLoader &rl)
	: _resource_heap(default_allocator(), "resource")
	, _loader(&rl)
	, _type_data(default_allocator())
	, _rm(default_allocator())
	, _autoload(false)
{
}

ResourceManager::~ResourceManager()
{
	auto cur = hash_map::begin(_rm);
	auto end = hash_map::end(_rm);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_rm, cur);

		const StringId64 type = cur->first.type;
		const StringId64 name = cur->first.name;
		on_offline(type, name);
		on_unload(type, cur->second.allocator, cur->second.data);
	}
}

bool ResourceManager::try_load(StringId64 package_name, StringId64 type, StringId64 name)
{
	ResourcePair id = { type, name };
	ResourceEntry &entry = hash_map::get(_rm, id, ResourceEntry::NOT_FOUND);

	if (entry == ResourceEntry::NOT_FOUND) {
		ResourceTypeData rtd;
		rtd.version = UINT32_MAX;
		rtd.load = NULL;
		rtd.online = NULL;
		rtd.offline = NULL;
		rtd.unload = NULL;
		rtd = hash_map::get(_type_data, type, rtd);

		ResourceRequest rr;
		rr.resource_manager = this;
		rr.package_name = package_name;
		rr.type = type;
		rr.name = name;
		rr.version = rtd.version;
		rr.load_function = rtd.load;
		rr.allocator = &_resource_heap;
		rr.data = NULL;

		return _loader->add_request(rr);
	}

	entry.references++;
	return true;
}

void ResourceManager::unload(StringId64 type, StringId64 name)
{
	ResourcePair id = { type, name };
	ResourceEntry &entry = hash_map::get(_rm, id, ResourceEntry::NOT_FOUND);

	if (--entry.references == 0) {
		on_offline(type, name);
		on_unload(type, entry.allocator, entry.data);

		hash_map::remove(_rm, id);
	}
}

void ResourceManager::reload(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	const ResourceEntry &entry = hash_map::get(_rm, id, ResourceEntry::NOT_FOUND);
	const u32 old_refs = entry.references;

	if (entry == ResourceEntry::NOT_FOUND)
		return;

	unload(type, name);
	while (!try_load(PACKAGE_RESOURCE_NONE, type, name)) {
		complete_requests();
	}

	ResourceEntry &new_entry = hash_map::get(_rm, id, ResourceEntry::NOT_FOUND);
	new_entry.references = old_refs;
}

bool ResourceManager::can_get(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	return _autoload ? true : hash_map::has(_rm, id);
}

const void *ResourceManager::get(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };

	CE_ASSERT(can_get(type, name), "Resource not loaded: " RESOURCE_ID_FMT, resource_id(type, name)._id);

	if (_autoload && !hash_map::has(_rm, id)) {
		while (!try_load(PACKAGE_RESOURCE_NONE, type, name)) {
			complete_requests();
		}

		while (!hash_map::has(_rm, id)) {
			complete_requests();
		}
	}

	const ResourceEntry &entry = hash_map::get(_rm, id, ResourceEntry::NOT_FOUND);
	return entry.data;
}

void ResourceManager::enable_autoload(bool enable)
{
	_autoload = enable;
}

void ResourceManager::complete_requests()
{
	ResourceRequest rr;
	while (_loader->_loaded.pop(rr)) {
		ResourceEntry entry;
		entry.references = 1;
		entry.data = rr.data;
		entry.allocator = rr.allocator;

		ResourcePair id = { rr.type, rr.name };

		hash_map::set(_rm, id, entry);

		on_online(rr.type, rr.name);
	}
}

void ResourceManager::register_type(StringId64 type, u32 version, LoadFunction load, UnloadFunction unload, OnlineFunction online, OfflineFunction offline)
{
	ResourceTypeData rtd;
	rtd.version = version;
	rtd.load = load;
	rtd.online = online;
	rtd.offline = offline;
	rtd.unload = unload;

	hash_map::set(_type_data, type, rtd);
}

void ResourceManager::on_online(StringId64 type, StringId64 name)
{
	OnlineFunction func = hash_map::get(_type_data, type, ResourceTypeData()).online;

	if (func)
		func(name, *this);
}

void ResourceManager::on_offline(StringId64 type, StringId64 name)
{
	OfflineFunction func = hash_map::get(_type_data, type, ResourceTypeData()).offline;

	if (func)
		func(name, *this);
}

void ResourceManager::on_unload(StringId64 type, Allocator *allocator, void *data)
{
	if (allocator == NULL)
		return;

	UnloadFunction func = hash_map::get(_type_data, type, ResourceTypeData()).unload;

	if (func)
		func(*allocator, data);
	else
		allocator->deallocate(data);
}

} // namespace crown
