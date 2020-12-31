/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/json/types.h"
#include "core/strings/types.h"

namespace crown
{
/// Functions to parse JSON-encoded data.
///
/// @ingroup JSON
namespace json
{
	/// Returns the data type of the JSON string @a json.
	JsonValueType::Enum type(const char* json);

	/// Returns the JSON number @a json as int.
	s32 parse_int(const char* json);

	/// Returns the JSON number @a json as f32.
	f32 parse_float(const char* json);

	/// Returns the JSON boolean @a json as bool.
	bool parse_bool(const char* json);

	/// Parses the JSON string @a json ad puts it into @a str.
	void parse_string(DynamicString& str, const char* json);

	/// Parses the JSON array @a json and puts it into @a arr as pointers to
	/// the corresponding items into the original @a json string.
	void parse_array(JsonArray& arr, const char* json);

	/// Parses the JSON object @a json and puts it into @a obj as map from
	/// key to pointer to the corresponding value into the original string @a json.
	void parse_object(JsonObject& obj, const char* json);

	/// Parses the JSON-encoded @a json.
	void parse(JsonObject& obj, const char* json);

	/// Parses the JSON-encoded @a json.
	void parse(JsonObject& obj, Buffer& json);

} // namespace json

} // namespace crown
