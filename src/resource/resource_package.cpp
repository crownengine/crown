/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "resource_package.h"
#include "resource_manager.h"
#include "package_resource.h"
#include "array.h"

namespace crown
{
ResourcePackage::ResourcePackage(StringId64 id, ResourceManager& resman)
	: _marker(MARKER)
	, _resman(&resman)
	, _id(id)
	, _package(NULL)
{
	resman.load(RESOURCE_TYPE_PACKAGE, _id);
	resman.flush();
	_package = (const PackageResource*)resman.get(RESOURCE_TYPE_PACKAGE, _id);
}

ResourcePackage::~ResourcePackage()
{
	_resman->unload(RESOURCE_TYPE_PACKAGE, _id);

	_marker = 0;
}

void ResourcePackage::load()
{
	for (u32 i = 0; i < array::size(_package->resources); ++i)
	{
		_resman->load(_package->resources[i].type, _package->resources[i].name);
	}
}

void ResourcePackage::unload()
{
	for (u32 i = 0; i < array::size(_package->resources); ++i)
	{
		_resman->unload(_package->resources[i].type, _package->resources[i].name);
	}
}

void ResourcePackage::flush()
{
	_resman->flush();
}

bool ResourcePackage::has_loaded() const
{
	for (u32 i = 0; i < array::size(_package->resources); ++i)
	{
		if (!_resman->can_get(_package->resources[i].type, _package->resources[i].name))
			return false;
	}

	return true;
}

} // namespace crown
