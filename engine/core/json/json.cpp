/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "json.h"
#include "container_types.h"
#include "string_utils.h"
#include "dynamic_string.h"
#include "map.h"

namespace crown
{
namespace json
{
	static const char* next(const char* str, const char c = 0)
	{
		CE_ASSERT_NOT_NULL(str);

		if (c && c != (*str))
		{
			CE_ASSERT(false, "Expected '%c' got '%c'", c, (*str));
		}

		return str + 1;
	}

	static const char* skip_whites(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		while ((*ch) && (*ch) <= ' ') ch = next(ch);

		return ch;
	}

	static const char* skip_comments(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);
		const char* ch = s;

		while ((*ch) == '/')
		{
			ch = next(ch, '/');
			ch = next(ch, '/');
			while ((*ch) && (*ch) != '\n') ch = next(ch);
			ch = skip_whites(ch);
		}

		return ch;
	}

	static const char* skip_string(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		bool escaped = false;

		if ((*ch) == '"')
		{
			while ((*(ch = next(ch))) != 0)
			{
				if ((*ch) == '"' && !escaped)
				{
					ch = next(ch);
					return ch;
				}
				else if ((*ch) == '\\') escaped = true;
				else escaped = false;
			}
		}

		return ch;
	}

	static const char* skip_number(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		while ((*ch) && (((*ch) >= '0' && (*ch) <= '9') ||
				(*ch) == '-' || (*ch) == '.' || (*ch) == '+' ||
				(*ch) == 'e' || (*ch) == 'E'))
		{
			ch = next(ch);
		}

		return ch;
	}

	static const char* skip_object(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		uint32_t brackets = 1;

		if ((*ch) == '{')
		{
			brackets++;
			ch = next(ch, '{');

			while ((*ch) && brackets != 1)
			{
				if ((*ch) == '}') brackets--;
				else if ((*ch) == '{') brackets++;
				ch = next(ch);
			}
		}

		return ch;
	}

	static const char* skip_array(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		uint32_t brackets = 1;

		if ((*ch) == '[')
		{
			brackets++;
			ch = next(ch, '[');

			while ((*ch) && brackets != 1)
			{
				if ((*ch) == ']') brackets--;
				else if ((*ch) == '[') brackets++;
				ch = next(ch);
			}
		}

		return ch;
	}

	static const char* skip_bool(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		switch ((*ch))
		{
			case 't':
			{
				ch = next(ch, 't');
				ch = next(ch, 'r');
				ch = next(ch, 'u');
				ch = next(ch, 'e');
				break;
			}
			case 'f':
			{
				ch = next(ch, 'f');
				ch = next(ch, 'a');
				ch = next(ch, 'l');
				ch = next(ch, 's');
				ch = next(ch, 'e');
				break;
			}
			default:
			{
				break;
			}
		}

		return ch;
	}

	static const char* skip_null(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;
		if ((*ch) == 'n')
		{
			ch = next(ch, 'n');
			ch = next(ch, 'u');
			ch = next(ch, 'l');
			ch = next(ch, 'l');
		}

		return ch;
	}

	static const char* skip_value(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;
		ch = skip_array(ch);
		ch = skip_object(ch);
		ch = skip_number(ch);
		ch = skip_string(ch);
		ch = skip_bool(ch);
		ch = skip_null(ch);

		return ch;
	}

	static bool is_escapee(char c)
	{
		return c == '"' || c == '\\' || c == '/' || c == '\b' || c == '\f' || c == '\n' ||
				c == '\r' || c == '\t';
	}

	JSONType::Enum type(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char c = s[0];

		switch (c)
		{
			case '{': return JSONType::OBJECT;
			case '[': return JSONType::ARRAY;
			case '"': return JSONType::STRING;
			case '-': return JSONType::NUMBER;
			default: return (c >= '0' && c <= '9') ? JSONType::NUMBER : (c == 'n' ? JSONType::NIL : JSONType::BOOL);
		}
	}

	void parse_string(const char* s, DynamicString& str)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		if ((*ch) == '"')
		{
			while ((*(ch = next(ch))))
			{
				// Empty string
				if ((*ch) == '"')
				{
					ch = next(ch);
					return;
				}
				else if ((*ch) == '\\')
				{
					ch = next(ch);

					if ((*ch) == 'u')
					{
						CE_FATAL("Not supported at the moment");
					}
					else if (is_escapee(*ch))
					{
						str += (*ch);
					}
					else
					{
						// Go to invalid string
						break;
					}
				}
				else
				{
					str += (*ch);
				}
			}
		}

		CE_FATAL("Bad string");
	}

	double parse_number(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

	 	Array<char> str(default_allocator());

		if ((*ch) == '-')
		{
			array::push_back(str, '-');
			ch = next(ch, '-');
		}
		while ((*ch) >= '0' && (*ch) <= '9')
		{
			array::push_back(str, (*ch));
			ch = next(ch);
		}

		if ((*ch) == '.')
		{
			array::push_back(str, '.');
			while ((*(ch = next(ch))) && (*ch) >= '0' && (*ch) <= '9')
			{
				array::push_back(str, *ch);
			}
		}

		if ((*ch) == 'e' || (*ch) == 'E')
		{
			array::push_back(str, *ch);
			ch = next(ch);

			if ((*ch) == '-' || (*ch) == '+')
			{
				array::push_back(str, *ch);
				ch = next(ch);
			}
			while ((*ch) >= '0' && (*ch) <= '9')
			{
				array::push_back(str, *ch);
				ch = next(ch);
			}
		}

		// Ensure null terminated
		array::push_back(str, '\0');

		return parse_double(array::begin(str));
	}

	bool parse_bool(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		switch(*ch)
		{
			case 't':
			{
				ch = next(ch, 't');
				ch = next(ch, 'r');
				ch = next(ch, 'u');
				ch = next(ch, 'e');
				return true;
			}
			case 'f':
			{
				ch = next(ch, 'f');
				ch = next(ch, 'a');
				ch = next(ch, 'l');
				ch = next(ch, 's');
				ch = next(ch, 'e');
				return false;
			}
			default:
			{
				CE_FATAL("Bad boolean");
				return false;
			}
		}
	}

	int32_t parse_int(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		return (int32_t) parse_number(s);
	}

	float parse_float(const char* s)
	{
		CE_ASSERT_NOT_NULL(s);

		return (float) parse_number(s);
	}

	void parse_array(const char* s, Array<const char*>& array)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		if ((*ch) == '[')
		{
			ch = next(ch, '[');

			// Skip whitespaces
			while ((*ch) && (*ch) <= ' ')
			{
				ch = next(ch);
			}

			if ((*ch) == ']')
			{
				ch = next(ch, ']');
				return;
			}

			while (*ch)
			{
				array::push_back(array, ch);

				ch = skip_value(ch);
				ch = skip_whites(ch);

				// Closing bracket (top-most array)
				if ((*ch) == ']')
				{
					ch = next(ch, ']');
					return;
				}

				// Skip until next ','
				ch = next(ch, ',');

				// Skip whites, eventually
				ch = skip_whites(ch);
			}
		}

		CE_FATAL("Bad array");
	}

	void parse_object(const char* s, Map<DynamicString, const char*>& object)
	{
		CE_ASSERT_NOT_NULL(s);

		const char* ch = s;

		if ((*ch) == '{')
		{
			ch = next(ch, '{');

			ch = skip_whites(ch);
			ch = skip_comments(ch);

			if ((*ch) == '}')
			{
				next(ch, '}');
				return;
			}

			while (*ch)
			{
				DynamicString key;
				parse_string(ch, key);

				ch = skip_string(ch);
				ch = skip_whites(ch);
				ch = next(ch, ':');
				ch = skip_whites(ch);

				map::set(object, key, ch);

				ch = skip_value(ch);
				ch = skip_whites(ch);
				ch = skip_comments(ch);
				
				if ((*ch) == '}')
				{
					next(ch, '}');
					return;
				}

				ch = next(ch, ',');
				ch = skip_whites(ch);
				ch = skip_comments(ch);
			}
		}

		CE_FATAL("Bad object");
	}
} // namespace json
} // namespace crown
