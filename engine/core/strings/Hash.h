/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Assert.h"
#include "Types.h"

namespace crown
{

//-----------------------------------------------------------------------------
struct Id32
{
	Id32() : key(0) {}

	const Id32& operator=(const Id32& other) { key = other.key; return *this; }
	const Id32& operator=(const uint32_t other) { key = other; return *this; }

	operator uint32_t() { return key; }

	uint32_t key;
};

//-----------------------------------------------------------------------------
struct Id64
{
	Id64() : key(0) {}

	const Id64& operator=(const Id64& other) { key = other.key; return *this; }
	const Id64& operator=(const uint64_t other) { key = other; return *this; }

	operator uint64_t()	{ return key; }

	uint64_t key;
};

/// String hashing.
namespace hash
{

// Constants
const uint32_t FNV1A_OFFSET_BASIS_32		= 2166136261u;
const uint32_t FNV1A_PRIME_32				= 16777619u;
const uint64_t FNV1A_OFFSET_BASIS_64		= 14695981039346656037ull;
const uint64_t FNV1A_PRIME_64				= 1099511628211ull;

// Functions
uint32_t murmur2_32(const void* key, size_t len, uint32_t seed);
uint64_t murmur2_64(const void* key, size_t len, unsigned int seed);
uint32_t fnv1a_32(const void* key, size_t len);
uint64_t fnv1a_64(const void* key, size_t len);

//-----------------------------------------------------------------------------
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
inline uint32_t murmur2_32(const void* key, size_t len, uint32_t seed)
{
	CE_ASSERT_NOT_NULL(key);

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
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
	        h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

//-----------------------------------------------------------------------------
inline uint64_t murmur2_64(const void* key, size_t len, unsigned int seed)
{
	CE_ASSERT_NOT_NULL(key);

	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h1 = seed ^ len;
	unsigned int h2 = 0;

	const unsigned int * data = (const unsigned int *)key;

	while(len >= 8)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		unsigned int k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if(len >= 4)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch(len)
	{
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
			h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	uint64_t h = h1;

	h = (h << 32) | h2;

	return h;
} 

//-----------------------------------------------------------------------------
/// FNV-1a hash, 32 bit
inline uint32_t fnv1a_32(const void* key, size_t len)
{
	CE_ASSERT(key != NULL, "Key must be != NULL");

	// FNV-1a
	uint32_t hash = FNV1A_OFFSET_BASIS_32;

	for (size_t i = 0; i < len; i++)
	{
		unsigned char* k = (unsigned char*)key;

		hash ^= k[i];
		hash *= FNV1A_PRIME_32;
	}

	return hash;
}

//-----------------------------------------------------------------------------
/// FNV-1a hash, 64 bit
inline uint64_t fnv1a_64(const void* key, size_t len)
{
	CE_ASSERT(key != NULL, "Key must be != NULL");

	// FNV-1a
	uint64_t hash = FNV1A_OFFSET_BASIS_64;

	for (size_t i = 0; i < len; i++)
	{
		unsigned char* k = (unsigned char*)key;
		
		hash ^= k[i];
		hash *= FNV1A_PRIME_64;
	}

	return hash;
}

} // namespace hash
} // namespace crown

