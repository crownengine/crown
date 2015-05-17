/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "types.h"
#include "config.h"
#include "macros.h"
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cctype>

namespace crown
{

inline size_t strlen(const char* str)
{
	return ::strlen(str);
}

inline const char* strstr(const char* str1, const char* str2)
{
	return ::strstr(str1, str2);
}

inline int32_t strcmp(const char* str1, const char* str2)
{
	return ::strcmp(str1, str2);
}

inline int32_t strncmp(const char* s1, const char* s2, size_t len)
{
	return ::strncmp(s1, s2, len);
}

inline char* strncpy(char* dest, const char* src, size_t len)
{
	char* ret = ::strncpy(dest, src, len);
	dest[len - 1] = '\0';

	return ret;
}

inline char* strcat(char* dest, const char* src)
{
	return ::strcat(dest, src);
}

inline char* strncat(char* dest, const char* src, size_t len)
{
	return ::strncat(dest, src, len);
}

inline int32_t vsnprintf(char* str, size_t num, const char* format, va_list args)
{
#if CROWN_COMPILER_MSVC
	int32_t len = _vsnprintf_s(str, num, _TRUNCATE, format, args);
	return (len == 1) ? _vscprintf(format, args) : len;
#else
	return ::vsnprintf(str, num, format, args);
#endif // CROWN_COMPILER_MSVC
}

inline int32_t snprintf(char* str, size_t n, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int32_t len = vsnprintf(str, n, format, args);
	va_end(args);
	return len;
}

inline const char* skip_spaces(const char* str)
{
	while (isspace(*str)) ++str;
	return str;
}

inline const char* skip_block(const char* str, char a, char b)
{
	uint32_t num = 0;

	for (char ch = *str++; *str != '\0'; ch = *str++)
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

inline const char* begin(const char* str)
{
	CE_ASSERT(str != NULL, "Str must be != NULL");

	return str;
}

inline const char* end(const char* str)
{
	CE_ASSERT(str != NULL, "Str must be != NULL");

	return str + strlen(str) + 1;
}

inline const char* find_first(const char* str, char c)
{
	CE_ASSERT(str != NULL, "Str must be != NULL");

	const char* str_begin = begin(str);

	while (str_begin != end(str))
	{
		if ((*str_begin) == c)
		{
			return str_begin;
		}

		str_begin++;
	}

	return end(str);
}

inline const char* find_last(const char* str, char c)
{
	CE_ASSERT(str != NULL, "Str must be != NULL");

	const char* str_end = end(str) - 1;

	while (str_end != begin(str) - 1)
	{
		if ((*str_end) == c)
		{
			return str_end;
		}

		str_end--;
	}

	return end(str);
}

inline void substring(const char* begin, const char* end, char* out, size_t len)
{
	CE_ASSERT(begin != NULL, "Begin must be != NULL");
	CE_ASSERT(end != NULL, "End must be != NULL");
	CE_ASSERT(out != NULL, "Out must be != NULL");

	size_t i = 0;

	char* out_iterator = out;

	while (begin != end && i < len)
	{
		(*out_iterator) = (*begin);

		begin++;
		out_iterator++;
		i++;
	}

	out[i] = '\0';
}

inline int32_t parse_int(const char* string)
{
	int val;
	int ok = sscanf(string, "%d", &val);

	CE_ASSERT(ok == 1, "Failed to parse int: %s", string);
	CE_UNUSED(ok);

	return val;
}

inline uint32_t parse_uint(const char* string)
{
	unsigned int val;
	int ok = sscanf(string, "%u", &val);

	CE_ASSERT(ok == 1, "Failed to parse uint: %s", string);
	CE_UNUSED(ok);

	return val;
}

inline float parse_float(const char* string)
{
	float val;
	int ok = sscanf(string, "%f", &val);

	CE_ASSERT(ok == 1, "Failed to parse float: %s", string);
	CE_UNUSED(ok);

	return val;
}

inline double parse_double(const char* string)
{
	double val;
	int ok = sscanf(string, "%lf", &val);

	CE_ASSERT(ok == 1, "Failed to parse float: %s", string);
	CE_UNUSED(ok);

	return val;
}

} // namespace crown
