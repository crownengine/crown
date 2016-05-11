/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "dynamic_string.h"
#include "error.h"
#include "murmur.h"
#include "string_id.h"
#include "string_utils.h"
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
	CE_ASSERT_NOT_NULL(str);
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
	CE_ASSERT_NOT_NULL(str);
	_id = murmur64(str, len, 0);
}

void StringId64::to_string(DynamicString& s)
{
	char buf[16+1];
	snprintf(buf, sizeof(buf), "%.16" PRIx64, _id);
	s.set(buf, sizeof(buf)-1);
}

} // namespace crown
