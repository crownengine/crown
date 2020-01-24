/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

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

	void hash(const char* str, u32 len);

	/// Parses the id from @a str.
	void parse(const char* str);

	/// Fills @a buf with the string representation of this id.
	const char* to_string(char* buf, u32 len) const;
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

	void hash(const char* str, u32 len);

	/// Parses the id from @a str.
	void parse(const char* str);

	/// Fills @a buf with the string representation of this id.
	const char* to_string(char* buf, u32 len) const;
};

} // namespace crown
