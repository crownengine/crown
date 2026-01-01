/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/functional.h"
#include "core/strings/string.inl"
#include "core/strings/string_id.h"

namespace crown
{
/// @addtogroup String
/// @{
inline StringId32::StringId32()
	: _id(0)
{
}

inline StringId32::StringId32(u32 idx)
	: _id(idx)
{
}

inline bool operator==(const StringId32 &a, const StringId32 &b)
{
	return a._id == b._id;
}

inline bool operator==(const StringId32 &a, const u32 b)
{
	return a._id == b;
}

inline bool operator!=(const StringId32 &a, const StringId32 &b)
{
	return a._id != b._id;
}

inline bool operator<(const StringId32 &a, const StringId32 &b)
{
	return a._id < b._id;
}

inline StringId64::StringId64()
	: _id(0)
{
}

inline StringId64::StringId64(u64 idx)
	: _id(idx)
{
}

inline bool operator==(const StringId64 &a, const StringId64 &b)
{
	return a._id == b._id;
}

inline bool operator==(const StringId64 &a, const u64 b)
{
	return a._id == b;
}

inline bool operator!=(const StringId64 &a, const StringId64 &b)
{
	return a._id != b._id;
}

inline bool operator<(const StringId64 &a, const StringId64 &b)
{
	return a._id < b._id;
}

template<>
struct hash<StringId32>
{
	u32 operator()(const StringId32 &id) const
	{
		return id._id;
	}
};

template<>
struct hash<StringId64>
{
	u32 operator()(const StringId64 &id) const
	{
		return (u32)id._id;
	}
};

#if CROWN_DEBUG && !CROWN_DEVELOPMENT
inline StringId32 STRING_ID_32(const char *str, const u32 id)
{
	StringId32 sid(str);
	CE_ASSERT(sid._id == id, "Hash mismatch");
	return sid;
}

inline StringId64 STRING_ID_64(const char *str, const u64 id)
{
	StringId64 sid(str);
	CE_ASSERT(sid._id = id, "Hash mismatch");
	return sid;
}
#endif
/// @}

} // namespace crown
