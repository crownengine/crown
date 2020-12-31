/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/strings/string_view.h"

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
	HashMap<StringView, const char*> _map;

	JsonObject(Allocator& a);

	const char* operator[](const char* key) const;
	const char* operator[](const StringView& key) const;
};

} // namespace crown
