/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/guid.h"
#include "core/json/types.h"
#include "core/math/types.h"
#include "core/strings/types.h"

namespace crown
{
/// Functions to parse SJSON-encoded data.
///
/// @ingroup JSON
namespace sjson
{
	/// Returns the data type of the SJSON string @a json.
	JsonValueType::Enum type(const char* json);

	/// Returns the SJSON number @a json as int.
	s32 parse_int(const char* json);

	/// Returns the SJSON number @a json as f32.
	f32 parse_float(const char* json);

	/// Returns the SJSON boolean @a json as bool.
	bool parse_bool(const char* json);

	/// Parses the SJSON string @a json ad puts it into @a str.
	void parse_string(DynamicString& str, const char* json);

	/// Parses the SJSON array @a json and puts it into @a arr as pointers to
	/// the corresponding items into the original @a json string.
	void parse_array(JsonArray& arr, const char* json);

	/// Parses the SJSON object @a json and puts it into @a obj as map from
	/// key to pointer to the corresponding value into the original string @a json.
	void parse_object(JsonObject& obj, const char* json);

	/// Parses the SJSON-encoded @a json.
	void parse(JsonObject& obj, const char* json);

	/// Parses the SJSON-encoded @a json.
	void parse(JsonObject& obj, Buffer& json);

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
	/// @note Quaternion = [x, y, z, w]
	Quaternion parse_quaternion(const char* json);

	/// Returns the array @a json as Matrix4x4.
	/// @note Matrix4x4 = [xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, tx, ty, tz, tw]
	Matrix4x4 parse_matrix4x4(const char* json);

	/// Returns the string @a json as StringId32.
	StringId32 parse_string_id(const char* json);

	/// Returns the string @a json as StringId64.
	StringId64 parse_resource_name(const char* json);

	/// Returns the string @a json as Guid.
	Guid parse_guid(const char* json);

	/// Parses the SJSON verbatim string @a json and puts it into @a string.
	void parse_verbatim(DynamicString& str, const char* json);

} // namespace sjson

} // namespace crown
