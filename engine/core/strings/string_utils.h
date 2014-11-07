/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include <cstdio>
#include <cstring>

#include "assert.h"
#include "types.h"
#include "config.h"
#include "macros.h"

namespace crown
{

inline size_t strlen(const char* str)
{
	return ::strlen(str);
}

inline const char* strstr(const char* str1, const char* str2)
{
	return ::strstr(str1, str2);
}

inline int32_t strcmp(const char* str1, const char* str2)
{
	return ::strcmp(str1, str2);
}

inline int32_t strncmp(const char* s1, const char* s2, size_t len)
{
	return ::strncmp(s1, s2, len);
}

inline char* strncpy(char* dest, const char* src, size_t len)
{
	char* ret = ::strncpy(dest, src, len);
	dest[len - 1] = '\0';

	return ret;
}

inline char* strcat(char* dest, const char* src)
{
	return ::strcat(dest, src);
}

inline char* strncat(char* dest, const char* src, size_t len)
{
	return ::strncat(dest, src, len);
}

inline const char* begin(const char* str)
{
	CE_ASSERT(str != NULL, "Str must be != NULL");

	return str;
}

inline const char* end(const char* str)
{
	CE_ASSERT(str != NULL, "Str must be != NULL");

	return str + strlen(str) + 1;
}

inline const char* find_first(const char* str, char c)
{
	CE_ASSERT(str != NULL, "Str must be != NULL");

	const char* str_begin = begin(str);

	while (str_begin != end(str))
	{
		if ((*str_begin) == c)
		{
			return str_begin;
		}

		str_begin++;
	}

	return end(str);
}

inline const char* find_last(const char* str, char c)
{
	CE_ASSERT(str != NULL, "Str must be != NULL");

	const char* str_end = end(str) - 1;

	while (str_end != begin(str) - 1)
	{
		if ((*str_end) == c)
		{
			return str_end;
		}

		str_end--;
	}

	return end(str);
}

inline void substring(const char* begin, const char* end, char* out, size_t len)
{
	CE_ASSERT(begin != NULL, "Begin must be != NULL");
	CE_ASSERT(end != NULL, "End must be != NULL");
	CE_ASSERT(out != NULL, "Out must be != NULL");

	size_t i = 0;

	char* out_iterator = out;

	while (begin != end && i < len)
	{
		(*out_iterator) = (*begin);

		begin++;
		out_iterator++;
		i++;
	}

	out[i] = '\0';
}

inline int32_t parse_int(const char* string)
{
	int val;
	int ok = sscanf(string, "%d", &val);

	CE_ASSERT(ok == 1, "Failed to parse int: %s", string);
	CE_UNUSED(ok);

	return val;
}

inline uint32_t parse_uint(const char* string)
{
	unsigned int val;
	int ok = sscanf(string, "%u", &val);

	CE_ASSERT(ok == 1, "Failed to parse uint: %s", string);
	CE_UNUSED(ok);

	return val;
}

inline float parse_float(const char* string)
{
	float val;
	int ok = sscanf(string, "%f", &val);

	CE_ASSERT(ok == 1, "Failed to parse float: %s", string);
	CE_UNUSED(ok);

	return val;
}

inline double parse_double(const char* string)
{
	double val;
	int ok = sscanf(string, "%lf", &val);

	CE_ASSERT(ok == 1, "Failed to parse float: %s", string);
	CE_UNUSED(ok);

	return val;
}

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
inline uint32_t murmur2_32(const void* key, size_t len, uint32_t seed = 0)
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

inline uint64_t murmur2_64(const void* key, int len, uint64_t seed = 0)
{
	const uint64_t m = 0xc6a4a7935bd1e995ull;
	const int r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t * data = (const uint64_t *)key;
	const uint64_t * end = data + (len/8);

	while(data != end)
	{
		uint64_t k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(len & 7)
	{
		case 7: h ^= uint64_t(data2[6]) << 48;
		case 6: h ^= uint64_t(data2[5]) << 40;
		case 5: h ^= uint64_t(data2[4]) << 32;
		case 4: h ^= uint64_t(data2[3]) << 24;
		case 3: h ^= uint64_t(data2[2]) << 16;
		case 2: h ^= uint64_t(data2[1]) << 8;
		case 1: h ^= uint64_t(data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

#ifdef CROWN_DEBUG
	inline uint32_t HASH32(const char *s, uint32_t value)
	{
		CE_ASSERT(murmur2_32(s, strlen(s), 0) == value, "Hash mismatch");
		return value;
	}

	inline uint64_t HASH64(const char* s, uint64_t value)
	{
		CE_ASSERT(murmur2_64(s, strlen(s), 0) == value, "Hash mismatch");
		return value;
	}
#else
	#define HASH32(s, v) (v)
	#define HASH64(s, v) (v)
#endif

} // namespace crown
