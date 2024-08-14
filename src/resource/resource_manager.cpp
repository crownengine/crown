/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
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

bool operator==(const ResourceManager::ResourceData &a, const ResourceManager::ResourceData &b)
{
	return a.references == b.references
		&& a.online_sequence_num == b.online_sequence_num
		&& a.allocator == b.allocator
		&& a.data == b.data
		;
}

bool operator!=(const ResourceManager::ResourceData &a, const ResourceManager::ResourceData &b)
{
	return !(a == b);
}

const ResourceManager::ResourceData ResourceManager::ResourceData::NOT_FOUND = { UINT32_MAX, 0u, NULL, NULL };

bool operator==(const ResourceManager::ResourceTypeData &a, const ResourceManager::ResourceTypeData &b)
{
	return a.version == b.version
		&& a.load == b.load
		&& a.online == b.online
		&& a.offline == b.offline
		&& a.unload == b.unload
		;
}

bool operator!=(const ResourceManager::ResourceTypeData &a, const ResourceManager::ResourceTypeData &b)
{
	return !(a == b);
}

const ResourceManager::ResourceTypeData ResourceManager::ResourceTypeData::NOT_FOUND = { UINT32_MAX, NULL, NULL, NULL, NULL };

template<>
struct hash<ResourceManager::ResourcePair>
{
	u32 operator()(const ResourceManager::ResourcePair &val) const
	{
		return u32(resource_id(val.type, val.name)._id);
	}
};

namespace resource_manager_internal
{
	void add_resource(ResourceManager &rm, StringId64 type, StringId64 name, Allocator *allocator, void *data)
	{
		ResourceManager::ResourceData rd;
		rd.references = 1;
		rd.online_sequence_num = 0;
		rd.allocator = allocator;
		rd.data = data;

		ResourceManager::ResourcePair id = { type, name };
		hash_map::set(rm._resources, id, rd);

		rm.on_online(type, name);
	}

} // namespace resource_manager_internal

ResourceManager::ResourceManager(ResourceLoader &rl)
	: _resource_heap(default_allocator(), "resource")
	, _loader(&rl)
	, _types(default_allocator())
	, _resources(default_allocator())
	, _autoload(false)
{
}

ResourceManager::~ResourceManager()
{
	auto cur = hash_map::begin(_resources);
	auto end = hash_map::end(_resources);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_resources, cur);

		const StringId64 type = cur->first.type;
		const StringId64 name = cur->first.name;
		on_offline(type, name);
		on_unload(type, cur->second.allocator, cur->second.data);
	}
}

bool ResourceManager::try_load(StringId64 package_name, StringId64 type, StringId64 name, u32 online_order)
{
	ResourcePair id = { type, name };
	ResourceData &rd = hash_map::get(_resources, id, ResourceData::NOT_FOUND);

	ResourceRequest rr;
	rr.resource_manager = this;
	rr.package_name = package_name;
	rr.type = type;
	rr.name = name;
	rr.online_order = online_order;
	rr.data = NULL;

	if (rd == ResourceData::NOT_FOUND) {
		ResourceTypeData rtd = hash_map::get(_types, type, ResourceTypeData::NOT_FOUND);
		CE_ENSURE(rtd != ResourceTypeData::NOT_FOUND);

		rr.allocator = &_resource_heap;
		rr.version = rtd.version;
		rr.load_function = rtd.load;
		return _loader->add_request(rr);
	}

	rd.references++;

	// Push a spurious loaded resource event. This avoids blocking forever
	// in complete_requests() by keeping the online_sequence_num updated.
	rr.allocator = NULL;
	rr.version = 0;
	rr.load_function = NULL;
	_loader->_loaded.push(rr);
	return true;
}

void ResourceManager::unload(StringId64 type, StringId64 name)
{
	ResourcePair id = { type, name };
	ResourceData &rd = hash_map::get(_resources, id, ResourceData::NOT_FOUND);

	if (--rd.references == 0) {
		on_offline(type, name);
		on_unload(type, rd.allocator, rd.data);

		hash_map::remove(_resources, id);
	}
}

void ResourceManager::reload(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	const ResourceData &rd = hash_map::get(_resources, id, ResourceData::NOT_FOUND);
	const u32 old_refs = rd.references;

	if (rd == ResourceData::NOT_FOUND)
		return;

	unload(type, name);

	while (!try_load(PACKAGE_RESOURCE_NONE, type, name, 0)) {
		complete_requests();
	}

	while (!hash_map::has(_resources, id)) {
		complete_requests();
	}

	ResourceData &new_rd = hash_map::get(_resources, id, ResourceData::NOT_FOUND);
	new_rd.references = old_refs;
}

bool ResourceManager::can_get(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	return _autoload ? true : hash_map::has(_resources, id);
}

const void *ResourceManager::get(StringId64 type, StringId64 name)
{
	CE_ASSERT(can_get(type, name), "Resource not loaded: " RESOURCE_ID_FMT, resource_id(type, name)._id);

	const ResourcePair id = { type, name };

	if (_autoload && !hash_map::has(_resources, id)) {
		while (!try_load(PACKAGE_RESOURCE_NONE, type, name, 0)) {
			complete_requests();
		}

		while (!hash_map::has(_resources, id)) {
			complete_requests();
		}
	}

	return hash_map::get(_resources, id, ResourceData::NOT_FOUND).data;
}

void ResourceManager::enable_autoload(bool enable)
{
	_autoload = enable;
}

void ResourceManager::complete_requests()
{
	ResourceRequest rr;
	while (_loader->_loaded.pop(rr)) {
		if (rr.type == RESOURCE_TYPE_PACKAGE || rr.type == RESOURCE_TYPE_CONFIG || _autoload) {
			// Always add packages and configs to the resource map because they never have
			// requirements and are never required by any resource, hence no online() order
			// constraints apply.
			resource_manager_internal::add_resource(*this
				, rr.type
				, rr.name
				, rr.allocator
				, rr.data
				);
		} else {
			ResourcePair rp { RESOURCE_TYPE_PACKAGE, rr.package_name };
			ResourceData &pkg_data = hash_map::get(_resources, rp, ResourceData::NOT_FOUND);
			CE_ENSURE(pkg_data != ResourceData::NOT_FOUND);

			if (rr.online_order != pkg_data.online_sequence_num) {
				// Cannot process this resource yet; we need to wait for all its requirements to be
				// put online() first. Put the request back into the loaded queue to try again
				// later.
				_loader->_loaded.push(rr);
			} else {
				++pkg_data.online_sequence_num;

				if (rr.data != NULL && rr.allocator != NULL) {
					// If this is a non-spurious request, add it to the resource map.
					resource_manager_internal::add_resource(*this
						, rr.type
						, rr.name
						, rr.allocator
						, rr.data
						);
				}
			}
		}
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

	hash_map::set(_types, type, rtd);
}

void ResourceManager::on_online(StringId64 type, StringId64 name)
{
	OnlineFunction func = hash_map::get(_types, type, ResourceTypeData::NOT_FOUND).online;

	if (func)
		func(name, *this);
}

void ResourceManager::on_offline(StringId64 type, StringId64 name)
{
	OfflineFunction func = hash_map::get(_types, type, ResourceTypeData::NOT_FOUND).offline;

	if (func)
		func(name, *this);
}

void ResourceManager::on_unload(StringId64 type, Allocator *allocator, void *data)
{
	if (allocator == NULL)
		return;

	UnloadFunction func = hash_map::get(_types, type, ResourceTypeData::NOT_FOUND).unload;

	if (func)
		func(*allocator, data);
	else
		allocator->deallocate(data);
}

} // namespace crown
