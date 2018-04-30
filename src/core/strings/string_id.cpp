/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.h"
#include "core/murmur.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string.h"
#include "core/strings/string_id.h"
#include <inttypes.h> // PRIx64

namespace crown
{
StringId32::StringId32(const char* str)
{
	hash(str, strlen32(str));
}

StringId32::StringId32(const char* str, u32 len)
{
	hash(str, len);
}

void StringId32::hash(const char* str, u32 len)
{
	CE_ENSURE(NULL != str);
	_id = murmur32(str, len, 0);
}

void StringId32::to_string(DynamicString& s)
{
	char buf[8+1];
	snprintf(buf, sizeof(buf), "%.8x", _id);
	s.set(buf, sizeof(buf)-1);
}

StringId64::StringId64(const char* str)
{
	hash(str, strlen32(str));
}

StringId64::StringId64(const char* str, u32 len)
{
	hash(str, len);
}

void StringId64::hash(const char* str, u32 len)
{
	CE_ENSURE(NULL != str);
	_id = murmur64(str, len, 0);
}

void StringId64::to_string(DynamicString& s)
{
	char buf[16+1];
	snprintf(buf, sizeof(buf), "%.16" PRIx64, _id);
	s.set(buf, sizeof(buf)-1);
}

} // namespace crown
