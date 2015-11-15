/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

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
	resman.load(PACKAGE_TYPE, _id);
	resman.flush();
	_package = (const PackageResource*)resman.get(PACKAGE_TYPE, _id);
}

ResourcePackage::~ResourcePackage()
{
	_resman->unload(PACKAGE_TYPE, _id);

	_marker = 0;
}

void ResourcePackage::load()
{
	for (uint32_t i = 0; i < array::size(_package->resources); ++i)
	{
		_resman->load(_package->resources[i].type, _package->resources[i].name);
	}
}

void ResourcePackage::unload()
{
	for (uint32_t i = 0; i < array::size(_package->resources); ++i)
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
	for (uint32_t i = 0; i < array::size(_package->resources); ++i)
	{
		if (!_resman->can_get(_package->resources[i].type, _package->resources[i].name))
			return false;
	}

	return true;
}

} // namespace crown
