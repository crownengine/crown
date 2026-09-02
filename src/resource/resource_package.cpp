/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/os.h"
#include "core/strings/string_id.inl"
#include "resource/package_resource.inl"
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
	, _num_resources_to_load(0)
{
}

ResourcePackage::~ResourcePackage()
{
	_resource_manager->unload(RESOURCE_TYPE_PACKAGE, _package_resource_name);
	_marker = 0;
}

void ResourcePackage::load()
{
	if (_package_resource != NULL || _num_resources_to_load != 0)
		return;

	_num_resources_to_load = 1;
	_resource_manager->load(this
		, RESOURCE_TYPE_PACKAGE
		, _package_resource_name
		, 0
		);
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
	return _package_resource != NULL && _num_resources_to_load == 0;
}

} // namespace crown
