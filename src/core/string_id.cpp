/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "types.h"
#include "murmur.h"
#include "string_utils.h"

namespace crown
{

StringId32::StringId32(const char* str)
	: _id(murmur32(str, strlen(str)))
{
}

StringId32::StringId32(const char* str, uint32_t len)
	: _id(murmur32(str, len))
{
}

StringId64::StringId64(const char* str)
	: _id(murmur64(str, strlen(str)))
{
}

StringId64::StringId64(const char* str, uint32_t len)
	: _id(murmur64(str, len))
{
}

} // namespace crown
