/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.h"
#include "core/murmur.h"
#include "core/strings/string.inl"
#include "core/strings/string_id.h"
#include <errno.h>
#include <inttypes.h> // PRIx64
#include <stdlib.h>   // strto*
#include <stb_sprintf.h>

namespace crown
{
StringId32::StringId32(const char *str)
{
	hash(str, strlen32(str));
}

StringId32::StringId32(const char *str, u32 len)
{
	hash(str, len);
}

void StringId32::hash(const char *str, u32 len)
{
	CE_ENSURE(NULL != str);
	_id = u32(murmur64(str, len, 0) & 0xffffffffu);
}

void StringId32::parse(const char *str)
{
	CE_ENSURE(NULL != str);
	errno = 0;
	_id = strtoul(str, NULL, 16);
	CE_ENSURE(errno != ERANGE && errno != EINVAL);
}

const char *StringId32::to_string(char *buf, u32 len) const
{
	stbsp_snprintf(buf, len, "%.8x", _id);
	return buf;
}

StringId64::StringId64(const char *str)
{
	hash(str, strlen32(str));
}

StringId64::StringId64(const char *str, u32 len)
{
	hash(str, len);
}

void StringId64::hash(const char *str, u32 len)
{
	CE_ENSURE(NULL != str);
	_id = murmur64(str, len, 0);
}

void StringId64::parse(const char *str)
{
	CE_ENSURE(NULL != str);
	errno = 0;
	_id = strtoull(str, NULL, 16);
	CE_ENSURE(errno != ERANGE && errno != EINVAL);
}

const char *StringId64::to_string(char *buf, u32 len) const
{
	stbsp_snprintf(buf, len, "%.16" PRIx64, _id);
	return buf;
}

} // namespace crown
