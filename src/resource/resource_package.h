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
	u32 _num_resources_queued;
	bool _package_resource_queued;
	bool _loaded;

	///
	ResourcePackage(StringId64 id, ResourceManager &resman);

	///
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
	bool has_loaded();
};

} // namespace crown
