/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

namespace crown
{
/// Header of a compiled navmesh resource.
///
/// @ingroup Resource
struct NavmeshResource
{
	u32 version;
	u32 size;
	u32 data_offset;
};

} // namespace crown
