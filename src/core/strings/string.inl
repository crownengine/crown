/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/error/error.inl"
#include "core/platform.h"
#include "core/types.h"
#include <ctype.h> // isspace
#include <stdarg.h>
#include <stdio.h> // sscanf, vsnprintf
#include <string.h>

namespace crown
{
inline s32 vsnprintf(char* str, size_t num, const char* format, va_list args)
{
#if CROWN_COMPILER_MSVC
	s32 len = _vsnprintf_s(str, num, _TRUNCATE, format, args);
	return (len == 1) ? _vscprintf(format, args) : len;
#else
	return ::vsnprintf(str, num, format, args);
#endif // CROWN_COMPILER_MSVC
}

inline s32 snprintf(char* str, size_t n, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	s32 len = vsnprintf(str, n, format, args);
	va_end(args);
	return len;
}

inline u32 strlen32(const char* str)
{
	return (u32)strlen(str);
}

inline const char* skip_spaces(const char* str)
{
	while (isspace(*str)) ++str;
	return str;
}

inline const char* skip_block(const char* str, char a, char b)
{
	u32 num = 0;

	for (char ch = *str++; ch != '\0'; ch = *str++)
	{
		if (ch == a) ++num;
		else if (ch == b)
		{
			if (--num == 0)
			{
				return str;
			}
		}
	}

	return NULL;
}

/// Returns pointer after EOL.
inline const char* strnl(const char* str)
{
	const char* eol = strchr(str, '\n');
	return eol ? eol + 1 : str + strlen(str);
}

// Written by Jack Handy: jakkhandy@hotmail.com
inline int wildcmp(const char *wild, const char *str)
{
	const char *cp = NULL, *mp = NULL;

	while (*str && *wild != '*')
	{
		if (*wild != *str && *wild != '?')
			return 0;
		++wild;
		++str;
	}

	while (*str)
	{
		if (*wild == '*')
		{
			if (!*++wild)
				return 1;
		  mp = wild;
		  cp = str + 1;
		}
		else if (*wild == *str || *wild == '?')
		{
			++wild;
			++str;
		}
		else
		{
			wild = mp;
			str = cp++;
		}
	}

	while (*wild == '*')
		++wild;

	return !*wild;
}

inline bool str_has_prefix(const char* str, const char* prefix)
{
	CE_ENSURE(NULL != str);
	CE_ENSURE(NULL != prefix);
	return strncmp(&str[0], prefix, strlen32(prefix)) == 0;
}

inline bool str_has_suffix(const char* str, const char* suffix)
{
	CE_ENSURE(NULL != str);
	CE_ENSURE(NULL != suffix);
	const u32 ml = strlen32(str);
	const u32 sl = strlen32(suffix);
	return sl <= ml && strncmp(&str[ml-sl], suffix, sl) == 0;
}

} // namespace crown
