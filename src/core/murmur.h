/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{
uint32_t murmur32(const void* key, uint32_t len, uint32_t seed = 0);
uint64_t murmur64(const void* key, uint32_t len, uint64_t seed = 0);
} // namespace crown
