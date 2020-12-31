/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/json/json.h"
#include "core/json/json_object.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"

namespace crown
{
namespace json
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
		case '"': json = skip_string(json); break;
		case '[': json = skip_block(json, '[', ']'); break;
		case '{': json = skip_block(json, '{', '}'); break;
		default: for (; *json != ',' && *json != '}' && *json != ']'; ++json) ; break;
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

				json = next(json, ',');
				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad array");
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
				parse_string(key, json);

				StringView fs_key(key_begin, key.length());

				json = skip_string(json);
				json = skip_spaces(json);
				json = next(json, ':');
				json = skip_spaces(json);

				hash_map::set(obj._map, fs_key, json);

				json = skip_value(json);
				json = skip_spaces(json);

				if (*json == '}')
					return;

				json = next(json, ',');
				json = skip_spaces(json);
			}
		}

		CE_FATAL("Bad object");
	}

	void parse(JsonObject& obj, const char* json)
	{
		CE_ENSURE(NULL != json);
		parse_object(obj, json);
	}

	void parse(JsonObject& obj, Buffer& json)
	{
		array::push_back(json, '\0');
		array::pop_back(json);
		parse(obj, array::begin(json));
	}

} // namespace json

} // namespace crown
