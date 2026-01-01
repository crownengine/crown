/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"
#include "core/strings/string.h"
#include <errno.h>
#include <stdlib.h> // strtod
#include <stb_sprintf.h>

namespace crown
{
namespace sjson
{
	static void default_error(const char *msg, void *user_data)
	{
		CE_UNUSED(user_data);
		CE_FATAL("%s", msg);
	}

	static thread_local SJsonError _error_function = default_error;
	static thread_local void *_error_user_data;

	void set_error_callback(SJsonError callback, void *user_data)
	{
		_error_function = callback;
		_error_user_data = user_data;
	}

	void vfatal(const char *format, va_list args)
	{
		char msg[1024];
		stbsp_vsnprintf(msg, sizeof(msg), format, args);
		_error_function(msg, _error_user_data);
	}

	void fatal(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		vfatal(format, args);
		va_end(args);
	}

#define NEXT_OR_RETURN(json, c, return_val)            \
	do {                                               \
		char _c = (c);                                 \
		if (_c && _c != *json) {                       \
			fatal("Expected '%c' got '%c'", _c, json); \
			return return_val;                         \
		}                                              \
	} while (0);                                       \
	++json

	static const char *skip_string(const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return NULL;
		}

		while (*++json) {
			if (*json == '"') {
				++json;
				return json;
			} else if (*json == '\\') {
				++json;
			}
		}

		return json;
	}

	static const char *skip_comments(const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return NULL;
		}

		if (*json == '/') {
			++json;
			if (*json == '/') {
				NEXT_OR_RETURN(json, '/', NULL);
				while (*json && *json != '\n')
					++json;
			} else if (*json == '*') {
				++json;
				while (*json && *json != '*')
					++json;
				NEXT_OR_RETURN(json, '*', NULL);
				NEXT_OR_RETURN(json, '/', NULL);
			} else {
				fatal("Bad comment");
				return NULL;
			}
		}

		return json;
	}

	static const char *skip_spaces(const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return NULL;
		}

		while (*json) {
			if (*json == '/')
				json = skip_comments(json);
			else if (isspace(*json) || *json == ',')
				++json;
			else
				break;
		}

		return json;
	}

	static const char *skip_value(const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return NULL;
		}

		switch (*json) {
		case '"':
			if (strncmp(json, "\"\"\"", 3) == 0) {
				json = strstr(json + 3, "\"\"\"");
				if (json == NULL) {
					fatal("Bad verbatim string");
					return NULL;
				}

				while (strncmp(json + 1, "\"\"\"", 3) == 0)
					++json;

				json += 3;
			} else {
				json = skip_string(json);
				if (*json == '"') {
					fatal("Bad string");
					return NULL;
				}
			}
			break;

		case '[':
		case '{': {
			u32 num = 0;
			char aa = *json;
			char bb = aa + 2; // Matching brace/bracket.

			while (*json != '\0') {
				if (*json == aa) {
					++json;
					++num;
				} else if (*json == bb) {
					++json;
					if (--num == 0)
						break;
				} else if (*json == '"') {
					json = skip_string(json);
					if (*json == '"') {
						json = strstr(json + 1, "\"\"\"");
						if (json == NULL) {
							fatal("Bad verbatim string");
							return NULL;
						}
						json += 3;
					}
				} else if (*json == '/') {
					json = skip_comments(json);
				} else {
					++json;
				}
			}
			break;
		}
		default: for (; *json != '\0' && *json != ',' && *json != '\n' && *json != ' ' && *json != '}' && *json != ']'; ++json); break;
		}

		return json;
	}

	JsonValueType::Enum type(const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return JsonValueType::NIL;
		}

		switch (*json) {
		case '"': return JsonValueType::STRING;
		case '{': return JsonValueType::OBJECT;
		case '[': return JsonValueType::ARRAY;
		case '-': return JsonValueType::NUMBER;
		default: return (isdigit(*json)) ? JsonValueType::NUMBER : (*json == 'n' ? JsonValueType::NIL : JsonValueType::BOOL);
		}
	}

	static const char *parse_key(const char *json, DynamicString &key)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return NULL;
		}

		if (*json == '"') {
			parse_string(key, json);
			return skip_string(json);
		}

		while (true) {
			if (isspace(*json) || *json == '=' || *json == ':')
				return json;

			key += *json++;
		}

		fatal("Bad object key '%s'", key.c_str());
		return NULL;
	}

	static f64 parse_number(const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return 0.0f;
		}

		TempAllocator512 alloc;
		Array<char> number(alloc);

		if (*json == '-') {
			array::push_back(number, '-');
			++json;
		}
		while (isdigit(*json)) {
			array::push_back(number, *json);
			++json;
		}

		if (*json == '.') {
			array::push_back(number, '.');
			while (*++json && isdigit(*json)) {
				array::push_back(number, *json);
			}
		}

		if (*json == 'e' || *json == 'E') {
			array::push_back(number, *json);
			++json;

			if (*json == '-' || *json == '+') {
				array::push_back(number, *json);
				++json;
			}
			while (isdigit(*json)) {
				array::push_back(number, *json);
				++json;
			}
		}

		if (array::size(number) == 0) {
			fatal("Invalid number");
			return 0.0f;
		}
		array::push_back(number, '\0');

		errno = 0;
		f64 val = strtod(array::begin(number), NULL);
		if (errno == ERANGE || errno == EINVAL) {
			fatal("Bad number '%s'", array::begin(number));
			return 0.0f;
		}
		return val;
	}

	s32 parse_int(const char *json)
	{
		return (s32)parse_number(json);
	}

	f32 parse_float(const char *json)
	{
		return (f32)parse_number(json);
	}

	bool parse_bool(const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return false;
		}

		switch (*json) {
		case 't':
			NEXT_OR_RETURN(json, 't', false);
			NEXT_OR_RETURN(json, 'r', false);
			NEXT_OR_RETURN(json, 'u', false);
			NEXT_OR_RETURN(json, 'e', false);
			--json;
			return true;

		case 'f':
			NEXT_OR_RETURN(json, 'f', false);
			NEXT_OR_RETURN(json, 'a', false);
			NEXT_OR_RETURN(json, 'l', false);
			NEXT_OR_RETURN(json, 's', false);
			NEXT_OR_RETURN(json, 'e', false);
			--json;
			return false;

		default:
			fatal("Bad boolean");
			return false;
		}
	}

	void parse_string(DynamicString &str, const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return;
		}

		if (*json == '"') {
			while (*++json) {
				if (*json == '"')
					return;

				if (*json == '\\') {
					++json;

					switch (*json) {
					case '"': str += '"'; break;
					case '\\': str += '\\'; break;
					case '/': str += '/'; break;
					case 'b': str += '\b'; break;
					case 'f': str += '\f'; break;
					case 'n': str += '\n'; break;
					case 'r': str += '\r'; break;
					case 't': str += '\t'; break;
					default: fatal("Bad escape character"); return; break;
					}
				} else {
					str += *json;
				}
			}
		}

		fatal("Bad string");
	}

	void parse_array(JsonArray &arr, const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return;
		}

		if (*json == '[') {
			json = skip_spaces(++json);

			if (*json == ']')
				return;

			while (*json) {
				array::push_back(arr, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == ']')
					return;

				json = skip_spaces(json);
			}
		}

		fatal("Bad array");
	}

	static void parse_root_object(JsonObject &obj, const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return;
		}

		while (*json) {
			const char *key_begin = *json == '"' ? (json + 1) : json;

			TempAllocator256 ta;
			DynamicString key(ta);
			json = parse_key(json, key);

			StringView fs_key(key_begin, key.length());

			json = skip_spaces(json);
			NEXT_OR_RETURN(json, (*json == '=') ? '=' : ':', /*void*/);
			json = skip_spaces(json);

			hash_map::set(obj._map, fs_key, json);

			json = skip_value(json);
			json = skip_spaces(json);
		}

		obj._end = json + 1;
	}

	void parse_object(JsonObject &obj, const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return;
		}

		if (*json == '{') {
			json = skip_spaces(++json);

			if (*json == '}') {
				obj._end = json + 1;
				return;
			}

			while (*json) {
				const char *key_begin = *json == '"' ? (json + 1) : json;

				TempAllocator256 ta;
				DynamicString key(ta);
				json = parse_key(json, key);

				StringView fs_key(key_begin, key.length());

				json = skip_spaces(json);
				NEXT_OR_RETURN(json, (*json == '=') ? '=' : ':', /*void*/);
				json = skip_spaces(json);

				hash_map::set(obj._map, fs_key, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == '}') {
					obj._end = json + 1;
					return;
				}

				json = skip_spaces(json);
			}
		}

		fatal("Bad object");
	}

	void parse(JsonObject &obj, const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return;
		}

		json = skip_spaces(json);

		if (*json == '{')
			parse_object(obj, json);
		else
			parse_root_object(obj, json);
	}

	void parse(JsonObject &obj, Buffer &json)
	{
		array::push_back(json, '\0');
		array::pop_back(json);
		parse(obj, array::begin(json));
	}

} // namespace sjson

namespace sjson
{
	Vector2 parse_vector2(const char *json)
	{
		TempAllocator64 ta;
		JsonArray arr(ta);
		sjson::parse_array(arr, json);

		Vector2 v;
		v.x = sjson::parse_float(arr[0]);
		v.y = sjson::parse_float(arr[1]);
		return v;
	}

	Vector3 parse_vector3(const char *json)
	{
		TempAllocator64 ta;
		JsonArray arr(ta);
		sjson::parse_array(arr, json);

		Vector3 v;
		v.x = sjson::parse_float(arr[0]);
		v.y = sjson::parse_float(arr[1]);
		v.z = sjson::parse_float(arr[2]);
		return v;
	}

	Vector4 parse_vector4(const char *json)
	{
		TempAllocator64 ta;
		JsonArray arr(ta);
		sjson::parse_array(arr, json);

		Vector4 v;
		v.x = sjson::parse_float(arr[0]);
		v.y = sjson::parse_float(arr[1]);
		v.z = sjson::parse_float(arr[2]);
		v.w = sjson::parse_float(arr[3]);
		return v;
	}

	Quaternion parse_quaternion(const char *json)
	{
		TempAllocator64 ta;
		JsonArray arr(ta);
		sjson::parse_array(arr, json);

		Quaternion q;
		q.x = sjson::parse_float(arr[0]);
		q.y = sjson::parse_float(arr[1]);
		q.z = sjson::parse_float(arr[2]);
		q.w = sjson::parse_float(arr[3]);
		return q;
	}

	Matrix4x4 parse_matrix4x4(const char *json)
	{
		TempAllocator256 ta;
		JsonArray arr(ta);
		sjson::parse_array(arr, json);

		Matrix4x4 m;
		m.x.x = sjson::parse_float(arr[ 0]);
		m.x.y = sjson::parse_float(arr[ 1]);
		m.x.z = sjson::parse_float(arr[ 2]);
		m.x.w = sjson::parse_float(arr[ 3]);

		m.y.x = sjson::parse_float(arr[ 4]);
		m.y.y = sjson::parse_float(arr[ 5]);
		m.y.z = sjson::parse_float(arr[ 6]);
		m.y.w = sjson::parse_float(arr[ 7]);

		m.z.x = sjson::parse_float(arr[ 8]);
		m.z.y = sjson::parse_float(arr[ 9]);
		m.z.z = sjson::parse_float(arr[10]);
		m.z.w = sjson::parse_float(arr[11]);

		m.t.x = sjson::parse_float(arr[12]);
		m.t.y = sjson::parse_float(arr[13]);
		m.t.z = sjson::parse_float(arr[14]);
		m.t.w = sjson::parse_float(arr[15]);
		return m;
	}

	StringId32 parse_string_id(const char *json)
	{
		TempAllocator256 ta;
		DynamicString str(ta);
		sjson::parse_string(str, json);
		return str.to_string_id();
	}

	StringId64 parse_resource_name(const char *json)
	{
		TempAllocator256 ta;
		DynamicString str(ta);
		sjson::parse_string(str, json);
		return StringId64(str.c_str());
	}

	Guid parse_guid(const char *json)
	{
		TempAllocator64 ta;
		DynamicString str(ta);
		sjson::parse_string(str, json);
		return guid::parse(str.c_str());
	}

	void parse_verbatim(DynamicString &str, const char *json)
	{
		if (json == NULL) {
			fatal("json is NULL");
			return;
		}

		NEXT_OR_RETURN(json, '"', /*void*/);
		NEXT_OR_RETURN(json, '"', /*void*/);
		NEXT_OR_RETURN(json, '"', /*void*/);

		const char *end = strstr(json, "\"\"\"");
		if (end == NULL) {
			fatal("Bad verbatim string");
			return;
		}

		// Find last '"'.
		end += 3;
		while (*end && *end == '"')
			++end;

		str.set(json, u32(end - 3 - json));
	}

} // namespace json

} // namespace crown
