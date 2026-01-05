/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/strings/dynamic_string.inl"

namespace crown
{
/// Reads a string line-by-line.
///
/// @ingroup String
struct LineReader
{
	const char *_str;
	const u32 _len;
	u32 _pos;

	explicit LineReader(const char *str)
		: _str(str)
		, _len(strlen32(str))
		, _pos(0)
	{
	}

	void read_line(DynamicString &line)
	{
		const char *s  = &_str[_pos];
		const char *nl = strnl(s);
		_pos += u32(nl - s);
		line.set(s, u32(nl - s));
	}

	bool eof() const
	{
		return _str[_pos] == '\0';
	}
};

} // namespace crown
