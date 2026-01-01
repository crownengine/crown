/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/error/error.inl"
#include "core/types.h"
#include <ctype.h>  // isspace
#include <string.h> // strncmp, strchr etc.

namespace crown
{
inline u32 strlen32(const char *str)
{
	return (u32)strlen(str);
}

inline const char *skip_spaces(const char *str)
{
	while (isspace(*str)) ++str;
	return str;
}

/// Returns pointer after EOL.
inline const char *strnl(const char *str)
{
	const char *eol = strchr(str, '\n');
	return eol ? eol + 1 : str + strlen(str);
}

inline bool str_has_prefix(const char *str, const char *prefix)
{
	CE_ENSURE(NULL != str);
	CE_ENSURE(NULL != prefix);
	return strncmp(&str[0], prefix, strlen32(prefix)) == 0;
}

inline bool str_has_suffix(const char *str, const char *suffix)
{
	CE_ENSURE(NULL != str);
	CE_ENSURE(NULL != suffix);
	const u32 ml = strlen32(str);
	const u32 sl = strlen32(suffix);
	return sl <= ml && strncmp(&str[ml - sl], suffix, sl) == 0;
}

} // namespace crown
