/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "string_types.h"
#include "types.h"

namespace crown
{
struct Guid
{
	u32 data1;
	u16 data2;
	u16 data3;
	u64 data4;
};

/// Functions to manipulate Guid.
///
/// @ingroup Core
namespace guid
{
	/// Returns a new randomly generated Guid.
	Guid new_guid();

	/// Parses the guid from @a str.
	Guid parse(const char* str);

	/// Parses the @a guid from @a str and returns true if success.
	bool try_parse(const char* str, Guid& guid);

	/// Fills @a s with the string representation of the @a guid.
	void to_string(const Guid& guid, DynamicString& s);
}

/// Returns whether Guid @a and @b are equal.
inline bool operator==(const Guid& a, const Guid& b)
{
	return a.data1 == b.data1
		&& a.data2 == b.data2
		&& a.data3 == b.data3
		&& a.data4 == b.data4
		;
}

const Guid GUID_ZERO = { 0u, 0u, 0u, 0u };

} // namespace crown
