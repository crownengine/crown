/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.h"
#include "core/murmur.h"
#include "core/strings/string.inl"
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

void StringId32::parse(const char* str)
{
	CE_ENSURE(NULL != str);
	int num = sscanf(str, "%8x", &_id);
	CE_ENSURE(num == 1);
	CE_UNUSED(num);
}

const char* StringId32::to_string(char* buf, u32 len) const
{
	snprintf(buf, len, "%.8x", _id);
	return buf;
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

void StringId64::parse(const char* str)
{
	u32 id[2];
	CE_ENSURE(NULL != str);
	int num = sscanf(str, "%8x%8x", &id[0], &id[1]);
	_id =  u64(id[0]) << 32;
	_id |= u64(id[1]) <<  0;
	CE_ENSURE(num == 2);
	CE_UNUSED(num);
}

const char* StringId64::to_string(char* buf, u32 len) const
{
	snprintf(buf, len, "%.16" PRIx64, _id);
	return buf;
}

} // namespace crown
