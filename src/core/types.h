/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "memory_types.h"
#include <cstddef>
#include <stdint.h>
#include <stdio.h>

namespace crown
{

struct StringId32
{
	uint32_t _id;

	StringId32() : _id(0) {}
	explicit StringId32(uint32_t idx) : _id(idx) {}
	explicit StringId32(const char* str);
	explicit StringId32(const char* str, uint32_t len);
	StringId32 operator=(StringId32 a) { _id = a._id; return *this; }
	bool operator==(StringId32 a) const { return _id == a._id; }
	bool operator!=(StringId32 a) const { return _id != a._id; }
	bool operator<(StringId32 a) const { return _id < a._id; }
	uint32_t id() const { return _id; }
	const char* to_string(char* buf);

	static const uint32_t STRING_LENGTH = 32;
};

struct StringId64
{
	uint64_t _id;

	StringId64() : _id(0) {}
	explicit StringId64(uint64_t idx) : _id(idx) {}
	explicit StringId64(const char* str);
	explicit StringId64(const char* str, uint32_t len);
	StringId64 operator=(StringId64 a) { _id = a._id; return *this; }
	bool operator==(StringId64 a) const { return _id == a._id; }
	bool operator!=(StringId64 a) const { return _id != a._id; }
	bool operator<(StringId64 a) const { return _id < a._id; }
	uint64_t id() const { return _id; }
	const char* to_string(char* buf);

	static const uint32_t STRING_LENGTH = 32;
};

typedef StringId64 ResourceId;

#define INVALID_ID 65535
#define ALLOCATOR_AWARE typedef int allocator_aware

/// Convert integer to type.
template <int v>
struct Int2Type { enum {value=v}; };

/// Determines if a class is allocator aware.
template <class T>
struct is_allocator_aware {

        template <typename C>
        static char test_fun(typename C::allocator_aware *);

        template <typename C>
        static int test_fun(...);

public:
        enum {
                value = (sizeof(test_fun<T>(0)) == sizeof(char))
        };
};

#define IS_ALLOCATOR_AWARE(T) is_allocator_aware<T>::value
#define IS_ALLOCATOR_AWARE_TYPE(T) Int2Type< IS_ALLOCATOR_AWARE(T) >

/// Allocator aware constuction
template <class T> inline T &construct(void *p, Allocator& a, Int2Type<true>) {new (p) T(a); return *(T *)p;}
template <class T> inline T &construct(void *p, Allocator& /*a*/, Int2Type<false>) {new (p) T; return *(T *)p;}
template <class T> inline T &construct(void *p, Allocator& a) {return construct<T>(p, a, IS_ALLOCATOR_AWARE_TYPE(T)());}

struct Id
{
	uint16_t id;
	uint16_t index;

	void decode(uint32_t id_and_index)
	{
		id = (id_and_index & 0xffff0000) >> 16;
		index = id_and_index & 0xffff;
	}

	uint32_t encode() const
	{
		return (uint32_t(id) << 16) | uint32_t(index);
	}

	bool operator==(const Id& other)
	{
		return id == other.id && index == other.index;
	}

	bool operator!=(const Id& other)
	{
		return id != other.id || index != other.index;
	}
};

} // namespace crown
