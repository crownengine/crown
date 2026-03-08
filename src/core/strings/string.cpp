/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/strings/string.h"
#include <ctype.h> // tolower

namespace crown
{
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
int wildcmp(const char *wild, const char *str)
{
	const char *cp = NULL, *mp = NULL;

	while (*str && *wild != '*') {
		if (*wild != *str && *wild != '?')
			return 0;
		++wild;
		++str;
	}

	while (*str) {
		if (*wild == '*') {
			if (!*++wild)
				return 1;
			mp = wild;
			cp = str + 1;
		} else if (*wild == *str || *wild == '?') {
			++wild;
			++str;
		} else {
			wild = mp;
			str = cp++;
		}
	}

	while (*wild == '*')
		++wild;

	return !*wild;
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

} // namespace crown
