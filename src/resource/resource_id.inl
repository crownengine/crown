/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/path.h"
#include "resource/resource_id.h"
#include <inttypes.h> // PRIx64

#define RESOURCE_ID_FMT "#ID(%.16" PRIx64 ")"
#define RESOURCE_ID_FMT_STR "#ID(%s)"
#define RESOURCE_ID_FMT_STR_PAIR "#ID(%s.%s)"
#define RESOURCE_ID_BUF_LEN 17

namespace crown
{
inline const char *resource_type(const char *path)
{
	return path::extension(path);
}

inline u32 resource_name_length(const char *type, const char *path)
{
	return u32(type - path - 1);
}

inline ResourceId resource_id(StringId64 type, StringId64 name)
{
	ResourceId id { type._id ^ name._id };
	return id;
}

inline ResourceId resource_id(const char *type, u32 type_len, const char *name, u32 name_len)
{
	return resource_id(StringId64(type, type_len), StringId64(name, name_len));
}

} // namespace crown
