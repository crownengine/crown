/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

namespace crown
{
u32 murmur32(const void *key, u32 len, u32 seed);
u64 murmur64(const void *key, u32 len, u64 seed);

} // namespace crown
