/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/functional.h"
#include "core/murmur.h"
#include "core/strings/string.inl"
#include "core/strings/string_id.h"

namespace crown
{
/// @addtogroup String
/// @{
inline bool operator==(const StringId32& a, const StringId32& b)
{
	return a._id == b._id;
}

inline bool operator==(const StringId32& a, const u32 b)
{
	return a._id == b;
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

inline bool operator==(const StringId64& a, const u64 b)
{
	return a._id == b;
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

#if CROWN_DEBUG && !CROWN_DEVELOPMENT
	inline u32 STRING_ID_32(const char* str, const u32 id)
	{
		CE_ASSERT(murmur32(str, strlen32(str), 0) == id, "Hash mismatch");
		return id;
	}

	inline u64 STRING_ID_64(const char* str, const u64 id)
	{
		CE_ASSERT(murmur64(str, strlen32(str), 0) == id, "Hash mismatch");
		return id;
	}
#endif
/// @}

} // namespace crown
