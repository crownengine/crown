/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/hash_map.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "resource/package_resource.h"
#include "resource/resource_id.inl"
#include "resource/resource_loader.h"
#include "resource/resource_manager.h"
#include "resource/resource_package.h"
#include "resource/simple_resource.h"

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
	, _resource_loader(&rl)
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

void ResourceManager::load(ResourcePackage *resource_package, StringId64 type, StringId64 name, u32 online_order)
{
	ResourcePair id = { type, name };
	ResourceData &rd = hash_map::get(_resources, id, ResourceData::NOT_FOUND);

	ResourceRequest rr;
	rr.resource_package = resource_package;
	rr.type = type;
	rr.name = name;
	rr.online_order = online_order;
	rr.data = NULL;

	if (rd == ResourceData::NOT_FOUND) {
		char buf[STRING_ID64_BUF_LEN];
		ResourceTypeData rtd = hash_map::get(_types, type, ResourceTypeData::NOT_FOUND);
		CE_ASSERT(rtd != ResourceTypeData::NOT_FOUND
			, "Unregistered resource type '%s'"
			, type.to_string(buf, sizeof(buf))
			);
		CE_UNUSED(buf);

		rr.allocator = &_resource_heap;
		rr.load_function = rtd.load;
		_resource_loader->add_request(rr);
		return;
	}

	rd.references++;

	// Push a spurious loaded resource event. This avoids blocking forever
	// in complete_requests() by keeping the online_sequence_num updated.
	rr.allocator = NULL;
	rr.load_function = NULL;
	_resource_loader->add_loaded(rr);
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

void *ResourceManager::reload(StringId64 type, StringId64 name)
{
	if (_resource_loader->_is_bundle) {
		CE_ASSERT(false, "Reloading resources in bundle mode is not supported");
		return NULL;
	}

	const ResourcePair id = { type, name };
	const ResourceData rd = hash_map::get(_resources, id, ResourceData::NOT_FOUND);

	if (rd == ResourceData::NOT_FOUND)
		return NULL;

	// Save old state.
	const u32 old_refs = rd.references;

	// Unload the old resource.
	on_offline(type, name);
	on_unload(type, rd.allocator, rd.data);
	hash_map::remove(_resources, id);

	// Load the new resource.
	load(NULL, type, name, 0);

	// Wait until the new resource has been loaded.
	while (!hash_map::has(_resources, id))
		complete_requests();

	// Restore old state into the new resource.
	ResourceData &new_rd = hash_map::get(_resources, id, ResourceData::NOT_FOUND);
	new_rd.references = old_refs;
	return new_rd.data;
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
		load(NULL, type, name, 0);

		while (!hash_map::has(_resources, id)) {
			complete_requests();
		}
	}

	return hash_map::get(_resources, id, ResourceData::NOT_FOUND).data;
}

File *ResourceManager::open_stream(StringId64 type, StringId64 name)
{
	CE_ASSERT(can_get(type, name), "Resource not loaded: " RESOURCE_ID_FMT, resource_id(type, name)._id);
	return _resource_loader->open_stream(type, name);
}

void ResourceManager::close_stream(File *stream)
{
	CE_ENSURE(stream != NULL);
	return _resource_loader->close_stream(stream);
}

void ResourceManager::enable_autoload(bool enable)
{
	_autoload = enable;
}

void ResourceManager::complete_requests()
{
	ResourceRequest rr;
	while (_resource_loader->pop_loaded(rr)) {
		ResourcePackage *package = rr.resource_package;
		const bool has_online_order = package != NULL
			&& rr.type != RESOURCE_TYPE_PACKAGE
			&& rr.type != RESOURCE_TYPE_CONFIG
			&& !_autoload
			;

		if (has_online_order) {
			ResourcePair rp { RESOURCE_TYPE_PACKAGE, package->_package_resource_name };
			ResourceData &pkg_data = hash_map::get(_resources, rp, ResourceData::NOT_FOUND);
			CE_ENSURE(pkg_data != ResourceData::NOT_FOUND);

			if (rr.online_order > pkg_data.online_sequence_num) {
				// Cannot process this resource yet; we need to wait for all its requirements to be
				// put online() first. Put the request back into the loaded queue to try again
				// later.
				_resource_loader->add_loaded(rr);
				continue;
			}

			++pkg_data.online_sequence_num;
		}

		if (!rr.is_spurious()) {
			resource_manager_internal::add_resource(*this
				, rr.type
				, rr.name
				, rr.allocator
				, rr.data
				);
		}

		if (package != NULL) {
			CE_ENSURE(package->_num_resources_to_load > 0);
			--package->_num_resources_to_load;

			if (package->_package_resource == NULL) {
				CE_ENSURE(rr.type == RESOURCE_TYPE_PACKAGE);
				CE_ENSURE(rr.name == package->_package_resource_name);
				package->_package_resource = (PackageResource *)get(RESOURCE_TYPE_PACKAGE, package->_package_resource_name);
				package->_num_resources_to_load = package->_package_resource->num_resources;

				for (u32 ii = 0; ii < package->_package_resource->num_resources; ++ii) {
					const ResourceOffset *ro = package_resource::resource_offset(package->_package_resource, ii);
					load(package
						, ro->type
						, ro->name
						, ro->online_order
						);
				}
			}
		}
	}
}

void ResourceManager::register_type(StringId64 type, u32 version, LoadFunction load, UnloadFunction unload, OnlineFunction online, OfflineFunction offline)
{
	CE_ASSERT(!hash_map::has(_types, type), "Type already registered");

	ResourceTypeData rtd;
	rtd.version = version;
	if (load == NULL) {
		if (type == RESOURCE_TYPE_PACKAGE || type == RESOURCE_TYPE_CONFIG) {
			rtd.load = simple_resource::load;
		} else {
			rtd.load = _resource_loader->_is_bundle
				? simple_resource::load_from_bundle
				: simple_resource::load
				;
		}
	} else {
		rtd.load = load;
	}

	if (unload == NULL) {
		if (type == RESOURCE_TYPE_PACKAGE || type == RESOURCE_TYPE_CONFIG) {
			rtd.unload = simple_resource::unload;
		} else {
			rtd.unload = _resource_loader->_is_bundle
				? simple_resource::unload_from_bundle
				: simple_resource::unload
				;
		}
	} else {
		rtd.unload = unload;
	}

	rtd.online = online;
	rtd.offline = offline;

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
	UnloadFunction func = hash_map::get(_types, type, ResourceTypeData::NOT_FOUND).unload;

	func(*allocator, data);
}

} // namespace crown
