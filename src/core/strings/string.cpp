/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/strings/string.h"

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

} // namespace crown
