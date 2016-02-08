/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{
/// Hashed string.
///
/// @ingroup String
struct StringId32
{
	u32 _id;

	StringId32() : _id(0) {}
	explicit StringId32(u32 idx) : _id(idx) {}
	explicit StringId32(const char* str);
	explicit StringId32(const char* str, u32 len);

	bool operator==(StringId32 a) const { return _id == a._id; }
	bool operator!=(StringId32 a) const { return _id != a._id; }
	bool operator<(StringId32 a) const { return _id < a._id; }

	const char* to_string(char* buf);

	static const u32 STRING_LENGTH = 32;
};

/// Hashed string.
///
/// @ingroup String
struct StringId64
{
	u64 _id;

	StringId64() : _id(0) {}
	explicit StringId64(u64 idx) : _id(idx) {}
	explicit StringId64(const char* str);
	explicit StringId64(const char* str, u32 len);

	bool operator==(StringId64 a) const { return _id == a._id; }
	bool operator!=(StringId64 a) const { return _id != a._id; }
	bool operator<(StringId64 a) const { return _id < a._id; }

	const char* to_string(char* buf);

	static const u32 STRING_LENGTH = 32;
};

typedef StringId64 ResourceId;

} // namespace crown
