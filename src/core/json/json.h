/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
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
struct JSONValueType
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

/// Functions to parse JSON-encoded strings.
///
/// @ingroup JSON
namespace json
{
	/// Returns the data type of the JSON string @a json.
	JSONValueType::Enum type(const char* json);

	/// Parses the JSON string @a json ad puts it into @a string.
	void parse_string(const char* json, DynamicString& string);

	/// Returns the JSON number @a json as double.
	double parse_number(const char* json);

	/// Returns the JSON number @a json as int.
	int32_t parse_int(const char* json);

	/// Returns the JSON number @a json as float.
	float parse_float(const char* json);

	/// Returns the JSON boolean @a json as bool.
	bool parse_bool(const char* json);

	/// Parses the JSON array @a json and puts it into @a array as pointers to
	/// the corresponding items into the original @a json string.
	void parse_array(const char* json, Array<const char*>& array);

	/// Parses the JSON object @a json and puts it into @a object as map from
	/// key to pointer to the corresponding value into the original string @a json.
	void parse_object(const char* json, Map<DynamicString, const char*>& object);
} // namespace json
} // namespace crown
