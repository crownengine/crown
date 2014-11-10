/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{
uint32_t murmur2_32(const void* key, size_t len, uint32_t seed = 0);
uint64_t murmur2_64(const void* key, int len, uint64_t seed = 0);
} // namespace crown
