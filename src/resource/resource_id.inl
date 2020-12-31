/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "resource/resource_id.h"

namespace crown
{
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
