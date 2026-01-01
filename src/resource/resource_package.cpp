/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/os.h"
#include "core/strings/string_id.inl"
#include "resource/package_resource.h"
#include "resource/resource_id.inl"
#include "resource/resource_manager.h"
#include "resource/resource_package.h"
#include "world/types.h"

namespace crown
{
ResourcePackage::ResourcePackage(StringId64 id, ResourceManager &resman)
	: _marker(RESOURCE_PACKAGE_MARKER)
	, _resource_manager(&resman)
	, _package_resource_name(id)
	, _package_resource(NULL)
	, _num_resources_queued(0)
	, _package_resource_queued(false)
	, _loaded(false)
{
}

ResourcePackage::~ResourcePackage()
{
	_resource_manager->unload(RESOURCE_TYPE_PACKAGE, _package_resource_name);
	_marker = 0;
}

void ResourcePackage::load()
{
	// Load the package resource itself.
	if (!_package_resource_queued) {
		_package_resource_queued = _resource_manager->try_load(PACKAGE_RESOURCE_NONE
			, RESOURCE_TYPE_PACKAGE
			, _package_resource_name
			, 0
			);
	} else {
		if (_package_resource == NULL) {
			if (!_resource_manager->can_get(RESOURCE_TYPE_PACKAGE, _package_resource_name)) {
				_resource_manager->complete_requests();
				return;
			}

			_package_resource = (PackageResource *)_resource_manager->get(RESOURCE_TYPE_PACKAGE, _package_resource_name);
		}

		// Now that the package resource has been loaded, issue loading requests for all the
		// resources it contains.
		for (u32 ii = _num_resources_queued; ii < _package_resource->num_resources; ++ii) {
			const ResourceOffset *ro = package_resource::resource_offset(_package_resource, ii);
			if (!_resource_manager->try_load(_package_resource_name, ro->type, ro->name, ro->online_order))
				break;

			++_num_resources_queued;
		}
	}
}

void ResourcePackage::unload()
{
	for (u32 ii = 0; ii < _package_resource->num_resources; ++ii) {
		const ResourceOffset *ro = package_resource::resource_offset(_package_resource, ii);
		_resource_manager->unload(ro->type, ro->name);
	}
}

void ResourcePackage::flush()
{
	while (!has_loaded()) {
		_resource_manager->complete_requests();
#if CROWN_PLATFORM_EMSCRIPTEN
		os::sleep(16);
#endif
	}
}

bool ResourcePackage::has_loaded()
{
	if (_loaded)
		return _loaded;

	load();

	if (_package_resource == NULL)
		return false;

	for (u32 ii = 0; ii < _package_resource->num_resources; ++ii) {
		const ResourceOffset *ro = package_resource::resource_offset(_package_resource, ii);
		if (!_resource_manager->can_get(ro->type, ro->name))
			return false;
	}

	_loaded = true;
	return _loaded;
}

} // namespace crown
