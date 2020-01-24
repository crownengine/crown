/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/functional.h"
#include "core/strings/string_id.h"

namespace crown
{
/// @addtogroup String
/// @{
inline bool operator==(const StringId32& a, const StringId32& b)
{
	return a._id == b._id;
}

inline bool operator!=(const StringId32& a, const StringId32& b)
{
	return a._id != b._id;
}

inline bool operator<(const StringId32& a, const StringId32& b)
{
	return a._id < b._id;
}

inline bool operator==(const StringId64& a, const StringId64& b)
{
	return a._id == b._id;
}

inline bool operator!=(const StringId64& a, const StringId64& b)
{
	return a._id != b._id;
}

inline bool operator<(const StringId64& a, const StringId64& b)
{
	return a._id < b._id;
}

template <>
struct hash<StringId32>
{
	u32 operator()(const StringId32& id) const
	{
		return id._id;
	}
};

template <>
struct hash<StringId64>
{
	u32 operator()(const StringId64& id) const
	{
		return (u32)id._id;
	}
};
/// @}

} // namespace crown
