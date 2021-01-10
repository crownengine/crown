/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/path.h"
#include "resource/resource_id.h"

namespace crown
{
inline const char* resource_type(const char* path)
{
	return path::extension(path);
}

inline u32 resource_name_length(const char* type, const char* path)
{
	return u32(type - path - 1);
}

inline ResourceId resource_id(StringId64 type, StringId64 name)
{
	ResourceId id { type._id ^ name._id };
	return id;
}

inline ResourceId resource_id(const char* type, u32 type_len, const char* name, u32 name_len)
{
	return resource_id(StringId64(type, type_len), StringId64(name, name_len));
}

} // namespace crown
