/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
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
uint64 murmur64(void* key, uint32 len, uint64 seed)
{
	uint64 m = 0xc6a4a7935bd1e995ull;
	int r = 47;

	uint64 h = seed ^ (len * m);

	uint64* data = (uint64*)key;
	uint64* end = data + (len/8);

	while (data != end) {
		uint64 k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	uint8* data2 = (uint8*)data;

	int i;
	uint32 len7 = len & 7;
	for (i = 0; i < len7; ++i) {
		uint32 idx = len7 - 1 - i;
		h ^= ((uint64)data2[idx]) << (idx << 3);
	}
	h *= i != 0 ? m : 1;

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

} /* namespace Crown */
