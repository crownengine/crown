/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/string_id.h"
#include <inttypes.h> // PRIx64

#define RESOURCE_ID_FMT "#ID(%.16" PRIx64 ")"
#define RESOURCE_ID_FMT_STR "#ID(%s)"
#define RESOURCE_ID_BUF_LEN 17

namespace crown
{
typedef StringId64 ResourceId;

/// Returns the resource id from @a type and @a name.
inline ResourceId resource_id(StringId64 type, StringId64 name);

/// Returns the resource id from @a type and @a name.
inline ResourceId resource_id(const char* type, u32 type_len, const char* name, u32 name_len);

/// Returns the resource id from @a path.
ResourceId resource_id(const char* path);

/// Returns the destination @a path of the resource @a id.
void destination_path(DynamicString& path, ResourceId id);

} // namespace crown
