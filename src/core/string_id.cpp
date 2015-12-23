/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "string_id.h"
#include "murmur.h"
#include "string_utils.h"
#include <inttypes.h> // PRIx64

namespace crown
{

StringId32::StringId32(const char* str)
	: _id(murmur32(str, strlen32(str)))
{
}

StringId32::StringId32(const char* str, uint32_t len)
	: _id(murmur32(str, len))
{
}

const char* StringId32::to_string(char* buf)
{
	snprintf(buf, STRING_LENGTH, "%.8x", _id);
	return buf;
}

StringId64::StringId64(const char* str)
	: _id(murmur64(str, strlen32(str)))
{
}

StringId64::StringId64(const char* str, uint32_t len)
	: _id(murmur64(str, len))
{
}

const char* StringId64::to_string(char* buf)
{
	snprintf(buf, STRING_LENGTH, "%.16" PRIx64, _id);
	return buf;
}

} // namespace crown
