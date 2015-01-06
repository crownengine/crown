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
struct NJSONType
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
	/// Returns the data type of the NJSON string @a str.
	NJSONType::Enum type(const char* str);

	/// Parses the NJSON string @a str ad puts it into @a string.
	void parse_string(const char* str, DynamicString& string);

	/// Returns the NJSON number @a str as double.
	double parse_number(const char* str);

	/// Returns the NJSON number @a str as int.
	int32_t parse_int(const char* str);

	/// Returns the NJSON number @a str as float.
	float parse_float(const char* str);

	/// Returns the NJSON boolean @a str as bool.
	bool parse_bool(const char* str);

	/// Parses the NJSON array @a str and puts it into @a array as pointers to
	/// the corresponding items into the original @a str string.
	void parse_array(const char* str, Array<const char*>& array);

	/// Parses the NJSON object @a str and puts it into @a object as map from
	/// key to pointer to the corresponding value into the original string @a str.
	void parse_object(const char* str, Map<DynamicString, const char*>& object);
} // namespace njson
} // namespace crown
