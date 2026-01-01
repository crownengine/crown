/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/strings/string_id.h"
#include "core/strings/types.h"

namespace crown
{
typedef StringId64 ResourceId;

/// Returns the type of the resource @a path.
inline const char *resource_type(const char *path);

/// Returns the resource name length of @a path. You must also pass the resource
/// @a type of @a path as returned by a previous call to resource_type(@a path).
///
/// The resource name always starts at the beginning of @a path.
inline u32 resource_name_length(const char *type, const char *path);

/// Returns the resource id from @a type and @a name.
inline ResourceId resource_id(StringId64 type, StringId64 name);

/// Returns the resource id from @a type and @a name.
inline ResourceId resource_id(const char *type, u32 type_len, const char *name, u32 name_len);

/// Returns the resource id from @a path.
ResourceId resource_id(const char *path);

/// Returns the destination @a path of the resource @a id.
void destination_path(DynamicString &path, ResourceId id);

/// Returns the stream destination @a path of the resource @a id.
void stream_destination_path(DynamicString &path, ResourceId id);

} // namespace crown
