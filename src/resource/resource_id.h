/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/string_id.h"
#include "core/strings/types.h"
#include <inttypes.h> // PRIx64

namespace crown
{
#define RESOURCE_ID "#ID(%.16" PRIx64 ")"

/// Returns the resource id from @a type and @a name.
inline ResourceId resource_id(StringId64 type, StringId64 name)
{
	ResourceId id { type._id ^ name._id };
	return id;
}

/// Returns the resource id from @a type and @a name.
inline ResourceId resource_id(const char* type, u32 type_len, const char* name, u32 name_len)
{
	return resource_id(StringId64(type, type_len), StringId64(name, name_len));
}

/// Returns the resource id from @a path.
ResourceId resource_id(const char* path);

/// Returns the destination @a path of the resource @a id.
void destination_path(DynamicString& path, ResourceId id);

} // namespace crown
