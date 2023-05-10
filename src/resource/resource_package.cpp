/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "resource/package_resource.h"
#include "resource/resource_manager.h"
#include "resource/resource_package.h"
#include "world/types.h"

namespace crown
{
ResourcePackage::ResourcePackage(StringId64 id, ResourceManager &resman)
	: _marker(RESOURCE_PACKAGE_MARKER)
	, _resource_manager(&resman)
	, _package_id(id)
	, _package(NULL)
{
}

ResourcePackage::~ResourcePackage()
{
	_resource_manager->unload(RESOURCE_TYPE_PACKAGE, _package_id);
	_marker = 0;
}

void ResourcePackage::load()
{
	_resource_manager->load(RESOURCE_TYPE_PACKAGE, _package_id);
	_resource_manager->flush();
	_package = (const PackageResource *)_resource_manager->get(RESOURCE_TYPE_PACKAGE, _package_id);

	for (u32 ii = 0; ii < _package->num_resources; ++ii) {
		const ResourceOffset *ro = package_resource::resource_offset(_package, ii);
		_resource_manager->load(ro->type, ro->name);
	}
}

void ResourcePackage::unload()
{
	for (u32 ii = 0; ii < _package->num_resources; ++ii) {
		const ResourceOffset *ro = package_resource::resource_offset(_package, ii);
		_resource_manager->unload(ro->type, ro->name);
	}
}

void ResourcePackage::flush()
{
	_resource_manager->flush();
}

bool ResourcePackage::has_loaded() const
{
	for (u32 ii = 0; ii < _package->num_resources; ++ii) {
		const ResourceOffset *ro = package_resource::resource_offset(_package, ii);
		if (!_resource_manager->can_get(ro->type, ro->name))
			return false;
	}

	return true;
}

} // namespace crown
