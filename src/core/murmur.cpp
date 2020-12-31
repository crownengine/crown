/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/murmur.h"

namespace crown
{
/// MurmurHash2, by Austin Appleby
///
/// @note
/// This code makes a few assumptions about how your machine behaves
///
/// 1. We can read a 4-byte value from any address without crashing
/// 2. sizeof(int) == 4
///
/// And it has a few limitations -
///
/// 1. It will not work incrementally.
/// 2. It will not produce the same results on little-endian and big-endian
///    machines.
u32 murmur32(const void* key, u32 len, u32 seed)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value
	unsigned int h = seed ^ len;

	// Mix 4 bytes at a time into the hash
	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array
	switch(len)
	{
		case 3: h ^= data[2] << 16; // Fallthrough
		case 2: h ^= data[1] << 8;  // Fallthrough
		case 1: h ^= data[0];       // Fallthrough
			h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

u64 murmur64(const void* key, u32 len, u64 seed)
{
	const u64 m = 0xc6a4a7935bd1e995ull;
	const int r = 47;

	u64 h = seed ^ (len * m);

	const u64 * data = (const u64 *)key;
	const u64 * end = data + (len/8);

	while(data != end)
	{
		u64 k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(len & 7)
	{
		case 7: h ^= u64(data2[6]) << 48; // Fallthrough
		case 6: h ^= u64(data2[5]) << 40; // Fallthrough
		case 5: h ^= u64(data2[4]) << 32; // Fallthrough
		case 4: h ^= u64(data2[3]) << 24; // Fallthrough
		case 3: h ^= u64(data2[2]) << 16; // Fallthrough
		case 2: h ^= u64(data2[1]) << 8;  // Fallthrough
		case 1: h ^= u64(data2[0]);       // Fallthrough
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

} // namespace crown
