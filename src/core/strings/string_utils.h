/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "error.h"
#include "platform.h"
#include "macros.h"
#include <stdio.h> // sscanf, vsnprintf
#include <string.h>
#include <stdarg.h>
#include <ctype.h> // isspace

namespace crown
{

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

inline uint32_t strlen32(const char* str)
{
	return (uint32_t)strlen(str);
}

inline const char* skip_spaces(const char* str)
{
	while (isspace(*str)) ++str;
	return str;
}

inline const char* skip_block(const char* str, char a, char b)
{
	uint32_t num = 0;

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

inline int32_t parse_int(const char* str)
{
	int val;
	int ok = sscanf(str, "%d", &val);
	CE_ASSERT(ok == 1, "Failed to parse int: %s", str);
	CE_UNUSED(ok);
	return val;
}

inline uint32_t parse_uint(const char* str)
{
	unsigned int val;
	int ok = sscanf(str, "%u", &val);
	CE_ASSERT(ok == 1, "Failed to parse uint: %s", str);
	CE_UNUSED(ok);
	return val;
}

inline float parse_float(const char* str)
{
	float val;
	int ok = sscanf(str, "%f", &val);
	CE_ASSERT(ok == 1, "Failed to parse float: %s", str);
	CE_UNUSED(ok);
	return val;
}

inline double parse_double(const char* str)
{
	double val;
	int ok = sscanf(str, "%lf", &val);
	CE_ASSERT(ok == 1, "Failed to parse float: %s", str);
	CE_UNUSED(ok);
	return val;
}

} // namespace crown
