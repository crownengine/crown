/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "dynamic_string.h"
#include "container_types.h"

namespace crown
{

/// @defgroup JSON

/// Enumerates JSON value types.
///
/// @ingroup JSON
struct JSONType
{
	enum Enum
	{
		OBJECT,
		ARRAY,
		STRING,
		NUMBER,
		BOOL,
		NIL
	};
};

/// Functions to parse JSON-encoded strings.
///
/// @ingroup JSON
namespace json
{
	/// Returns the data type of the JSON string @a s.
	JSONType::Enum type(const char* s);

	/// Parses the JSON string @a s ad puts it into @a str.
	void parse_string(const char* s, DynamicString& str);

	/// Returns the JSON number @a s as double.
	double parse_number(const char* s);

	/// Returns the JSON number @a s as int.
	int32_t parse_int(const char* s);

	/// Returns the JSON number @a s as float.
	float parse_float(const char* s);

	/// Returns the JSON boolean @a s as bool.
	bool parse_bool(const char* s);

	/// Parses the JSON array @a s and puts it into @a array as pointers to
	/// the corresponding items into the original @a s string.
	void parse_array(const char* s, Array<const char*>& array);

	/// Parses the JSON object @a s and puts it into @a object as map from
	/// key to pointer to the corresponding value into the original string @a s.
	void parse_object(const char* s, Map<DynamicString, const char*>& object);
} // namespace json
} // namespace crown
