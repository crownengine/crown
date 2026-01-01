/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/guid.h"

namespace crown
{
inline bool operator==(const Guid &a, const Guid &b)
{
	return a.data1 == b.data1
		&& a.data2 == b.data2
		;
}

inline bool operator<(const Guid &a, const Guid &b)
{
	if (a.data1 != b.data1)
		return a.data1 < b.data1;
	if (a.data2 != b.data2)
		return a.data2 < b.data2;

	return false;
}

template<>
struct hash<Guid>
{
	u32 operator()(const Guid &id) const
	{
		return u32(id.data1 ^ id.data2);
	}
};

} // namespace crown
