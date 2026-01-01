/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/murmur.h"
#include <string.h>

namespace crown
{
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
// Note - This code makes a few assumptions about how your machine behaves -
//
// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
//
// And it has a few limitations -
//
// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.
u64 murmur64(const void *key, u32 len, u64 seed)
{
	const u64 m = 0xc6a4a7935bd1e995ull;
	const int r = 47;

	u64 h = seed ^ (len * m);

	const u64 *data = (u64 *)key;
	const u64 *end = data + (len/8);

	while (data != end) {
#if (CROWN_PLATFORM_ANDROID || CROWN_PLATFORM_EMSCRIPTEN) && CROWN_ARCH_32BIT
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
