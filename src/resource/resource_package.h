/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"

namespace crown
{
/// Collection of resources to load in a batch.
struct ResourcePackage
{
	u32 _marker;
	ResourceManager *_resource_manager;
	StringId64 _package_resource_name;
	const PackageResource *_package_resource;
	u32 _num_resources_to_load;

	///
	ResourcePackage(StringId64 id, ResourceManager &resman);

	///
	~ResourcePackage();

	/// Starts loading all the resources in the package asynchronously. The function returns before
	/// the resources have been loaded. Call has_loaded() to poll for completion or flush() to wait
	/// for completion.
	void load();

	/// Unloads all the resources in the package.
	void unload();

	/// Waits until the package has been loaded.
	void flush();

	/// Returns whether the package has been loaded.
	bool has_loaded();
};

} // namespace crown
