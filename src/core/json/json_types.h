/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "fixed_string.h"

namespace crown
{

/// @defgroup JSON

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

typedef Array<const char*> JsonArray;
typedef Map<FixedString, const char*> JsonObject;

} // namespace crown
