/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

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

} // namespace crown
