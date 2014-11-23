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
	/// Returns the data type of the JSON string @a str.
	JSONType::Enum type(const char* str);

	/// Parses the JSON string @a str ad puts it into @a string.
	void parse_string(const char* str, DynamicString& string);

	/// Returns the JSON number @a str as double.
	double parse_number(const char* str);

	/// Returns the JSON number @a str as int.
	int32_t parse_int(const char* str);

	/// Returns the JSON number @a str as float.
	float parse_float(const char* str);

	/// Returns the JSON boolean @a str as bool.
	bool parse_bool(const char* str);

	/// Parses the JSON array @a str and puts it into @a array as pointers to
	/// the corresponding items into the original @a str string.
	void parse_array(const char* str, Array<const char*>& array);

	/// Parses the JSON object @a str and puts it into @a object as map from
	/// key to pointer to the corresponding value into the original string @a str.
	void parse_object(const char* str, Map<DynamicString, const char*>& object);
} // namespace json
} // namespace crown
