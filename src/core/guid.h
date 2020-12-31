/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

#define GUID_BUF_LEN 37

namespace crown
{
/// Holds a globally unique identifier.
///
/// @ingroup Core
struct Guid
{
	u32 data1;
	u16 data2;
	u16 data3;
	u64 data4;
};

namespace guid_globals
{
	///
	void init();

	///
	void shutdown();

} // namespace guid_globals

/// Functions to manipulate Guid.
///
/// @ingroup Core
namespace guid
{
	/// Returns a new randomly generated Guid.
	/// @note User must call guid_globals::init() first.
	Guid new_guid();

	/// Parses the guid from @a str.
	Guid parse(const char* str);

	/// Parses the @a guid from @a str and returns true if success.
	bool try_parse(Guid& guid, const char* str);

	/// Returns @a guid converted to ASCIIZ.
	/// @a buf size must be greater than or equal to GUID_BUF_LEN or the
	/// returned string will be truncated.
	const char* to_string(char* buf, u32 len, const Guid& guid);

} // namespace guid

/// Returns whether Guid @a and @b are equal.
bool operator==(const Guid& a, const Guid& b);

/// Returns whether Guid @a is lesser than @b.
bool operator<(const Guid& a, const Guid& b);

template <typename T>
struct hash;

template<>
struct hash<Guid>
{
	u32 operator()(const Guid& id) const;
};

static const Guid GUID_ZERO = { 0u, 0u, 0u, 0u };

} // namespace crown
