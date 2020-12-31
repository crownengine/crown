/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/guid.h"

namespace crown
{
inline bool operator==(const Guid& a, const Guid& b)
{
	return a.data1 == b.data1
		&& a.data2 == b.data2
		&& a.data3 == b.data3
		&& a.data4 == b.data4
		;
}

inline bool operator<(const Guid& a, const Guid& b)
{
	if (a.data1 != b.data1)
		return a.data1 < b.data1;
	if (a.data2 != b.data2)
		return a.data2 < b.data2;
	if (a.data3 != b.data3)
		return a.data3 < b.data3;
	if (a.data4 != b.data4)
		return a.data4 < b.data4;

	return false;
}

inline u32 hash<Guid>::operator()(const Guid& id) const
{
	return id.data1;
}

} // namespace crown
