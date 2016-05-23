/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "array.h"
#include "package_resource.h"
#include "resource_manager.h"
#include "resource_package.h"
#include "world_types.h"

namespace crown
{
ResourcePackage::ResourcePackage(StringId64 id, ResourceManager& resman)
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
	_package = (const PackageResource*)_resource_manager->get(RESOURCE_TYPE_PACKAGE, _package_id);

	for (u32 i = 0; i < array::size(_package->resources); ++i)
	{
		_resource_manager->load(_package->resources[i].type, _package->resources[i].name);
	}
}

void ResourcePackage::unload()
{
	for (u32 i = 0; i < array::size(_package->resources); ++i)
	{
		_resource_manager->unload(_package->resources[i].type, _package->resources[i].name);
	}
}

void ResourcePackage::flush()
{
	_resource_manager->flush();
}

bool ResourcePackage::has_loaded() const
{
	for (u32 i = 0; i < array::size(_package->resources); ++i)
	{
		if (!_resource_manager->can_get(_package->resources[i].type, _package->resources[i].name))
			return false;
	}

	return true;
}

} // namespace crown
