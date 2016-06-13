/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "fixed_string.h"

/// @defgroup JSON
/// @ingroup Core
namespace crown
{
/// Enumerates JSON value types.
///
/// @ingroup JSON
struct JsonValueType
{
	enum Enum
	{
		NIL,
		BOOL,
		NUMBER,
		STRING,
		ARRAY,
		OBJECT
	};
};

/// Array of pointers to json-encoded data.
///
/// @ingroup JSON
typedef Array<const char*> JsonArray;

/// Map from key to pointers to json-encoded data.
///
/// @ingroup JSON
struct JsonObject
{
	Map<FixedString, const char*> _map;

	JsonObject(Allocator& a);

	const char* operator[](const char* key) const;
	const char* operator[](const FixedString& key) const;
};

} // namespace crown
