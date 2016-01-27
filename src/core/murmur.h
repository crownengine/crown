/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{
u32 murmur32(const void* key, u32 len, u32 seed);
u64 murmur64(const void* key, u32 len, u64 seed);
} // namespace crown
