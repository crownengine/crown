/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

#define STRING_ID32_BUF_LEN 9
#define STRING_ID64_BUF_LEN 17

namespace crown
{
/// Hashed string.
///
/// @ingroup String
struct StringId32
{
	u32 _id;

	StringId32() : _id(0) {}
	StringId32(u32 idx) : _id(idx) {}
	explicit StringId32(const char* str);
	explicit StringId32(const char* str, u32 len);

	void hash(const char* str, u32 len);

	/// Parses the id from @a str.
	void parse(const char* str);

	/// Returns this string converted to ASCIIZ.
	/// @a buf size must be greater than or equal to STRING_ID32_BUF_LEN or the
	/// returned string will be truncated.
	const char* to_string(char* buf, u32 len) const;
};

/// Hashed string.
///
/// @ingroup String
struct StringId64
{
	u64 _id;

	StringId64() : _id(0) {}
	StringId64(u64 idx) : _id(idx) {}
	explicit StringId64(const char* str);
	explicit StringId64(const char* str, u32 len);

	void hash(const char* str, u32 len);

	/// Parses the id from @a str.
	void parse(const char* str);

	/// Returns this string converted to ASCIIZ.
	/// @a buf size must be greater than or equal to STRING_ID64_BUF_LEN or the
	/// returned string will be truncated.
	const char* to_string(char* buf, u32 len) const;
};

} // namespace crown
