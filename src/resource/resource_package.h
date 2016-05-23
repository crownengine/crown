/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "resource_types.h"
#include "string_id.h"
#include "types.h"

namespace crown
{
/// Collection of resources to load in a batch.
struct ResourcePackage
{
	u32 _marker;
	ResourceManager* _resource_manager;
	StringId64 _package_id;
	const PackageResource* _package;

public:

	ResourcePackage(StringId64 id, ResourceManager& resman);
	~ResourcePackage();

	/// Loads all the resources in the package.
	/// @note
	/// The resources are not immediately available after the call is made,
	/// instead, you have to poll for completion with has_loaded()
	void load();

	/// Unloads all the resources in the package.
	void unload();

	/// Waits until the package has been loaded.
	void flush();

	/// Returns whether the package has been loaded.
	bool has_loaded() const;
};

} // namespace crown
