/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "json_types.h"
#include "math_types.h"
#include "dynamic_string.h"

namespace crown
{

/// Functions to parse SJSON-encoded strings.
///
/// @ingroup JSON
namespace sjson
{
	/// Returns the data type of the SJSON string @a json.
	JsonValueType::Enum type(const char* json);

	/// Parses the SJSON string @a json ad puts it into @a string.
	void parse_string(const char* json, DynamicString& string);

	/// Returns the SJSON number @a json as double.
	double parse_number(const char* json);

	/// Returns the SJSON number @a json as int.
	int32_t parse_int(const char* json);

	/// Returns the SJSON number @a json as float.
	float parse_float(const char* json);

	/// Returns the SJSON boolean @a json as bool.
	bool parse_bool(const char* json);

	/// Parses the SJSON array @a json and puts it into @a array as pointers to
	/// the corresponding items into the original @a json string.
	void parse_array(const char* json, JsonArray& array);

	/// Parses the SJSON object @a json and puts it into @a object as map from
	/// key to pointer to the corresponding value into the original string @a json.
	void parse_object(const char* json, Map<DynamicString, const char*>& object);

	/// Parses the SJSON-encoded @a json.
	void parse(const char* json, Map<DynamicString, const char*>& object);

	/// Parses the SJSON object @a json and puts it into @a object as map from
	/// key to pointer to the corresponding value into the original string @a json.
	void parse_object(const char* json, JsonObject& object);

	/// Parses the SJSON-encoded @a json.
	void parse(const char* json, JsonObject& object);

	/// Parses the SJSON-encoded @a json.
	void parse(Buffer& json, JsonObject& object);
} // namespace sjson

namespace sjson
{
	/// Returns the array @a json as Vector2.
	/// @note Vector2 = [x, y]
	Vector2 parse_vector2(const char* json);

	/// Returns the array @a json as Vector3.
	/// @note Vector3 = [x, y, z]
	Vector3 parse_vector3(const char* json);

	/// Returns the array @a json as Vector4.
	/// @note Vector4 = [x, y, z, w]
	Vector4 parse_vector4(const char* json);

	/// Returns the array @a json as Quaternion.
	/// @note Quaternion = [x, y, z, angle]
	Quaternion parse_quaternion(const char* json);

	/// Returns the array @a json as Matrix4x4.
	/// @note Matrix4x4 = [xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, tx, ty, tz, tw]
	Matrix4x4 parse_matrix4x4(const char* json);

	/// Returns the string @a json as StringId32.
	StringId32 parse_string_id(const char* json);

	/// Returns the string @a json as ResourceId.
	ResourceId parse_resource_id(const char* json);
} // namespace sjson

} // namespace crown
