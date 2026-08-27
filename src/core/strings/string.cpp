/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/functional.h"
#include "core/strings/string.h"
#include "core/strings/string_view.inl"
#include <ctype.h> // tolower
#include <errno.h>
#include <stdlib.h>
#include <string.h> // memchr

namespace crown
{
const char *find(const StringView &str, char c)
{
	return (const char *)memchr(str.data(), c, str.length());
}

const char *find_reverse(const StringView &str, char c)
{
	for (u32 i = str.length(); i > 0; --i) {
		if (str.data()[i - 1] == c)
			return str.data() + i - 1;
	}

	return NULL;
}

const char *skip_block(const char *str, char a, char b)
{
	u32 num = 0;

	for (char ch = *str++; ch != '\0'; ch = *str++) {
		if (ch == a) {
			++num;
		} else if (ch == b) {
			if (--num == 0)
				return str;
		}
	}

	return NULL;
}

// Written by Jack Handy: jakkhandy@hotmail.com
int wildcmp(const StringView &wild, const StringView &str)
{
	u32 wild_i = 0;
	u32 str_i = 0;
	u32 star_i = UINT32_MAX;
	u32 match_i = 0;

	while (str_i < str.length()) {
		if (wild_i < wild.length()
			&& (wild.data()[wild_i] == '?' || wild.data()[wild_i] == str.data()[str_i])
			) {
			++wild_i;
			++str_i;
		} else if (wild_i < wild.length() && wild.data()[wild_i] == '*') {
			star_i = wild_i++;
			match_i = str_i;
		} else if (star_i != UINT32_MAX) {
			wild_i = star_i + 1;
			str_i = ++match_i;
		} else {
			return 0;
		}
	}

	while (wild_i < wild.length() && wild.data()[wild_i] == '*')
		++wild_i;

	return wild_i == wild.length();
}

int wildcmp(const char *wild, const char *str)
{
	return wildcmp(StringView(wild), StringView(str));
}

int strncasecmp(const char *str1, const char *str2, u32 len)
{
	CE_ENSURE(NULL != str1);
	CE_ENSURE(NULL != str2);

	for (u32 i = 0; i < len; ++i) {
		const char c1 = str1[i];
		const char c2 = str2[i];
		const int d = tolower(c1) - tolower(c2);
		if (d != 0 || c1 == '\0' || c2 == '\0')
			return d;
	}

	return 0;
}

int from_hex(s64 &val, const char *hex)
{
	errno = 0;
	val = strtol(hex, NULL, 16);
	return !(errno != ERANGE && errno != EINVAL);
}

} // namespace crown
