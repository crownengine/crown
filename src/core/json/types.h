/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
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
typedef Array<const char *> JsonArray;

/// Map from key to pointers to json-encoded data.
///
/// @ingroup JSON
struct JsonObject
{
	HashMap<StringView, const char *> _map;
	const char *_end; ///< Points to the character following the the object's last character.

	///
	explicit JsonObject(Allocator &a);

	///
	const char *operator[](const char *key) const;

	///
	const char *operator[](const StringView &key) const;
};

/// Map from flattened key to pointers to json-encoded data.
///
/// @ingroup JSON
typedef HashMap<DynamicString, const char *> FlatJsonObject;

} // namespace crown
