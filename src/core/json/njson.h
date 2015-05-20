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
struct NJSONValueType
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

/// Functions to parse NJSON-encoded strings.
///
/// @ingroup JSON
namespace njson
{
	/// Returns the data type of the NJSON string @a json.
	NJSONValueType::Enum type(const char* json);

	/// Parses the NJSON string @a json ad puts it into @a string.
	void parse_string(const char* json, DynamicString& string);

	/// Returns the NJSON number @a json as double.
	double parse_number(const char* json);

	/// Returns the NJSON number @a json as int.
	int32_t parse_int(const char* json);

	/// Returns the NJSON number @a json as float.
	float parse_float(const char* json);

	/// Returns the NJSON boolean @a json as bool.
	bool parse_bool(const char* json);

	/// Parses the NJSON array @a json and puts it into @a array as pointers to
	/// the corresponding items into the original @a json string.
	void parse_array(const char* json, Array<const char*>& array);

	/// Parses the NJSON object @a json and puts it into @a object as map from
	/// key to pointer to the corresponding value into the original string @a json.
	void parse_object(const char* json, Map<DynamicString, const char*>& object);

	/// Parses the NJSON-encoded @a json.
	void parse(const char* json, Map<DynamicString, const char*>& object);
} // namespace njson
} // namespace crown
