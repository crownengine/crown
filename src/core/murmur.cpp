/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/murmur.h"
#include <string.h>

namespace crown
{
u64 murmur64(const void *key, u32 len, u64 seed)
{
	const u64 m = 0xc6a4a7935bd1e995ull;
	const int r = 47;

	u64 h = seed ^ (len * m);

	const u64 *data = (const u64 *)key;
	const u64 *end = data + (len/8);

	while (data != end) {
#if CROWN_PLATFORM_ANDROID && CROWN_ARCH_32BIT
		u64 k;
		memcpy(&k, (const unsigned char *)data++, sizeof(k));
#else
		u64 k = *data++;
#endif

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char *data2 = (const unsigned char *)data;

	switch (len & 7) {
	case 7: h ^= u64(data2[6]) << 48; // Fallthrough
	case 6: h ^= u64(data2[5]) << 40; // Fallthrough
	case 5: h ^= u64(data2[4]) << 32; // Fallthrough
	case 4: h ^= u64(data2[3]) << 24; // Fallthrough
	case 3: h ^= u64(data2[2]) << 16; // Fallthrough
	case 2: h ^= u64(data2[1]) << 8;  // Fallthrough
	case 1: h ^= u64(data2[0]);       // Fallthrough
		h *= m;
	}

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

} // namespace crown
