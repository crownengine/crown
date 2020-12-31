/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"

namespace crown
{
namespace sjson
{
	static const char* next(const char* json, const char c = 0)
	{
		CE_ENSURE(NULL != json);

		if (c && c != *json)
		{
			CE_ASSERT(false, "Expected '%c' got '%c'", c, *json);
		}

		return ++json;
	}

	static const char* skip_string(const char* json)
	{
		CE_ENSURE(NULL != json);

		while (*++json)
		{
			if (*json == '"')
			{
				++json;
				return json;
			}
			else if (*json == '\\')
			{
				++json;
			}
		}

		return json;
	}

	static const char* skip_value(const char* json)
	{
		CE_ENSURE(NULL != json);

		switch (*json)
		{
		case '"':
			json = skip_string(json);
			if (*json == '"')
			{
				++json;
				json = strstr(json, "\"\"\"");
				CE_ENSURE(json);
				++json;
				++json;
				++json;
			}
			break;

		case '[': json = skip_block(json, '[', ']'); break;
		case '{': json = skip_block(json, '{', '}'); break;
		default: for (; *json != '\0' && *json != ',' && *json != '\n' && *json != ' ' && *json != '}' && *json != ']'; ++json) ; break;
		}

		return json;
	}

	static const char* skip_comments(const char* json)
	{
		CE_ENSURE(NULL != json);

		if (*json == '/')
		{
			++json;
			if (*json == '/')
			{
				json = next(json, '/');
				while (*json && *json != '\n')
					++json;
			}
			else if (*json == '*')
			{
				++json;
				while (*json && *json != '*')
					++json;
				json = next(json, '*');
				json = next(json, '/');
			}
			else
				CE_FATAL("Bad comment");
		}

		return json;
	}

	static const char* skip_spaces(const char* json)
	{
		CE_ENSURE(NULL != json);

		while (*json)
		{
			if (*json == '/') json = skip_comments(json);
			else if (isspace(*json) || *json == ',') ++json;
			else break;
		}

		return json;
	}

	JsonValueType::Enum type(const char* json)
	{
		CE_ENSURE(NULL != json);

		switch (*json)
		{
		case '"': return JsonValueType::STRING;
		case '{': return JsonValueType::OBJECT;
		case '[': return JsonValueType::ARRAY;
		case '-': return JsonValueType::NUMBER;
		default: return (isdigit(*json)) ? JsonValueType::NUMBER : (*json == 'n' ? JsonValueType::NIL : JsonValueType::BOOL);
		}
	}

	static const char* parse_key(const char* json, DynamicString& key)
	{
		CE_ENSURE(NULL != json);
		if (*json == '"')
		{
			parse_string(key, json);
			return skip_string(json);
		}

		while (true)
		{
			if (isspace(*json) || *json == '=' || *json == ':')
				return json;

			key += *json++;
		}

		CE_FATAL("Bad key");
		return NULL;
	}

	static f64 parse_number(const char* json)
	{
		CE_ENSURE(NULL != json);

		TempAllocator512 alloc;
	 	Array<char> number(alloc);

		if (*json == '-')
		{
			array::push_back(number, '-');
			++json;
		}
		while (isdigit(*json))
		{
			array::push_back(number, *json);
			++json;
		}

		if (*json == '.')
		{
			array::push_back(number, '.');
			while (*++json && isdigit(*json))
			{
				array::push_back(number, *json);
			}
		}

		if (*json == 'e' || *json == 'E')
		{
			array::push_back(number, *json);
			++json;

			if (*json == '-' || *json == '+')
			{
				array::push_back(number, *json);
				++json;
			}
			while (isdigit(*json))
			{
				array::push_back(number, *json);
				++json;
			}
		}

		array::push_back(number, '\0');

		f64 val;
		int ok = sscanf(array::begin(number), "%lf", &val);
		CE_ASSERT(ok == 1, "Failed to parse f64: %s", array::begin(number));
		CE_UNUSED(ok);
		return val;
	}

	s32 parse_int(const char* json)
	{
		return (s32)parse_number(json);
	}

	f32 parse_float(const char* json)
	{
		return (f32)parse_number(json);
	}

	bool parse_bool(const char* json)
	{
		CE_ENSURE(NULL != json);

		switch (*json)
		{
		case 't':
			json = next(json, 't');
			json = next(json, 'r');
			json = next(json, 'u');
			next(json, 'e');
			return true;

		case 'f':
			json = next(json, 'f');
			json = next(json, 'a');
			json = next(json, 'l');
			json = next(json, 's');
			next(json, 'e');
			return false;

		default:
			CE_FATAL("Bad boolean");
			return false;
		}
	}

	void parse_string(DynamicString& str, const char* json)
	{
		CE_ENSURE(NULL != json);

		if (*json == '"')
		{
			while (*++json)
			{
				if (*json == '"')
					return;

				if (*json == '\\')
				{
					++json;

					switch (*json)
					{
					case '"': str += '"'; break;
					case '\\': str += '\\'; break;
					case '/': str += '/'; break;
					case 'b': str += '\b'; break;
					case 'f': str += '\f'; break;
					case 'n': str += '\n'; break;
					case 'r': str += '\r'; break;
					case 't': str += '\t'; break;
					default: CE_FATAL("Bad escape character"); break;
					}
				}
				else
				{
					str += *json;
				}
			}
		}

		CE_FATAL("Bad string");
	}

	void parse_array(JsonArray& arr, const char* json)
	{
		CE_ENSURE(NULL != json);

		if (*json == '[')
		{
			json = skip_spaces(++json);

			if (*json == ']')
				return;

			while (*json)
			{
				array::push_back(arr, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == ']')
					return;

				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad array");
	}

	static void parse_root_object(JsonObject& obj, const char* json)
	{
		CE_ENSURE(NULL != json);

		while (*json)
		{
			const char* key_begin = *json == '"' ? (json + 1) : json;

			TempAllocator256 ta;
			DynamicString key(ta);
			json = parse_key(json, key);

			StringView fs_key(key_begin, key.length());

			json = skip_spaces(json);
			json = next(json, (*json == '=') ? '=' : ':');
			json = skip_spaces(json);

			hash_map::set(obj._map, fs_key, json);

			json = skip_value(json);
			json = skip_spaces(json);
		}
	}

	void parse_object(JsonObject& obj, const char* json)
	{
		CE_ENSURE(NULL != json);

		if (*json == '{')
		{
			json = skip_spaces(++json);

			if (*json == '}')
				return;

			while (*json)
			{
				const char* key_begin = *json == '"' ? (json + 1) : json;

				TempAllocator256 ta;
				DynamicString key(ta);
				json = parse_key(json, key);

				StringView fs_key(key_begin, key.length());

				json = skip_spaces(json);
				json = next(json, (*json == '=') ? '=' : ':');
				json = skip_spaces(json);

				hash_map::set(obj._map, fs_key, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == '}')
					return;

				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad object");
	}

	void parse(JsonObject& obj, const char* json)
	{
		CE_ENSURE(NULL != json);

		json = skip_spaces(json);

		if (*json == '{')
			parse_object(obj, json);
		else
			parse_root_object(obj, json);
	}

	void parse(JsonObject& obj, Buffer& json)
	{
		array::push_back(json, '\0');
		array::pop_back(json);
		parse(obj, array::begin(json));
	}

} // namespace sjson

namespace sjson
{
	Vector2 parse_vector2(const char* json)
	{
		TempAllocator64 ta;
		JsonArray arr(ta);
		sjson::parse_array(arr, json);

		Vector2 v;
		v.x = sjson::parse_float(arr[0]);
		v.y = sjson::parse_float(arr[1]);
		return v;
	}

	Vector3 parse_vector3(const char* json)
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

	Vector4 parse_vector4(const char* json)
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

	Quaternion parse_quaternion(const char* json)
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

	Matrix4x4 parse_matrix4x4(const char* json)
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

	StringId32 parse_string_id(const char* json)
	{
		TempAllocator256 ta;
		DynamicString str(ta);
		sjson::parse_string(str, json);
		return str.to_string_id();
	}

	StringId64 parse_resource_name(const char* json)
	{
		TempAllocator256 ta;
		DynamicString str(ta);
		sjson::parse_string(str, json);
		return StringId64(str.c_str());
	}

	Guid parse_guid(const char* json)
	{
		TempAllocator64 ta;
		DynamicString str(ta);
		sjson::parse_string(str, json);
		return guid::parse(str.c_str());
	}

	void parse_verbatim(DynamicString& str, const char* json)
	{
		CE_ENSURE(NULL != json);

		json = next(json, '"');
		json = next(json, '"');
		json = next(json, '"');

		const char* end = strstr(json, "\"\"\"");
		CE_ASSERT(end, "Bad verbatim string");

		str.set(json, u32(end - json));
	}

} // namespace json

} // namespace crown
