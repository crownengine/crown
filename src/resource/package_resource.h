/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/strings/types.h"
#include "resource/types.h"

namespace crown
{
struct ResourceOffset
{
	StringId64 type;
	StringId64 name;
	u32 offset;      ///< Relative offset from package_resource::data().
	u32 size;
	u32 online_order;
	u32 _pad;
};

struct PackageResource
{
	u32 version;
	u32 num_resources;
	// ResourceOffset offsets[num_resources]
	// Data (16-bytes aligned)
};

namespace package_resource
{
	///
	const ResourceOffset *resource_offset(const PackageResource *pr, u32 index);

	/// Returns a pointer to the data segment of the package resource @a pr.
	const u8 *data(const PackageResource *pr);

} // namespace package_resource

} // namespace crown
