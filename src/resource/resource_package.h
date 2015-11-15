/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "resource_types.h"

namespace crown
{

/// Collection of resources to load in a batch.
struct ResourcePackage
{
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

public:

	enum { MARKER = 0x9a1ac68c };

private:

	uint32_t _marker;

	ResourceManager* _resman;
	StringId64 _id;
	const PackageResource* _package;
};

} // namespace crown
