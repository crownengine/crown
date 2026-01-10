/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/memory/memory.inl"
#include "resource/package_resource.h"

namespace crown
{
namespace package_resource
{
	const ResourceOffset *resource_offset(const PackageResource *pr, u32 index)
	{
		const ResourceOffset *ro = (ResourceOffset *)(pr + 1);
		return ro + index;
	}

	const u8 *data(const PackageResource *pr)
	{
		const u8 *data_offset = (u8 *)resource_offset(pr, pr->num_resources);
		return (u8 *)memory::align_top(data_offset, 16);
	}

} // namespace package_resource

} // namespace crown
