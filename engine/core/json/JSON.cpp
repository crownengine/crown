/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Assert.h"
#include "JSON.h"
#include "Types.h"
#include "List.h"
#include "TempAllocator.h"
#include <cstdio>

namespace crown
{

//-----------------------------------------------------------------------------
static const char* next(const char* str, const char c = 0)
{
	CE_ASSERT_NOT_NULL(str);

	if (c && c != (*str))
	{
		CE_ASSERT(false, "Expected '%c' got '%c'", c, (*str));
	}

	return str + 1;
}

//-----------------------------------------------------------------------------
static const char* skip_whites(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	const char* ch = s;

	while ((*ch) && (*ch) <= ' ') ch = next(ch);

	return ch;
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
static bool is_escapee(char c)
{
	return c == '"' || c == '\\' || c == '/' || c == '\b' || c == '\f' || c == '\n' ||
			c == '\r' || c == '\t';
}

//-----------------------------------------------------------------------------
JSONType JSON::type(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	switch (s[0])
	{
		case '{': return JT_OBJECT;
		case '[': return JT_ARRAY;
		case '"': return JT_STRING;
		case '-': return JT_NUMBER;
		default: return s[0] >= '0' && s[0] <= '9' ? JT_NUMBER : (s[0] == 'n' ? JT_NIL : JT_BOOL);
	}
}


//-----------------------------------------------------------------------------
void JSON::parse_string(const char* s, List<char>& str)
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
				str.push_back('\0');
				return;
			}
			else if ((*ch) == '\\')
			{
				ch = next(ch);

				if ((*ch) == 'u')
				{
					CE_ASSERT(false, "Not supported at the moment");
				}
				else if (is_escapee(*ch))
				{
					str.push_back('\\');
					str.push_back(*ch);
				}
				else
				{
					// Go to invalid string
					break;
				}
			}
			else
			{
				str.push_back(*ch);
			}
		}
	}

	CE_ASSERT(false, "Not a valid string");
}

//-----------------------------------------------------------------------------
double JSON::parse_number(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	const char* ch = s;

	TempAllocator1024 allocator;
 	List<char> str(allocator);

	if ((*ch) == '-')
	{
		str.push_back('-');
		ch = next(ch, '-');
	}
	while ((*ch) >= '0' && (*ch) <= '9')
	{
		str.push_back((*ch));
		ch = next(ch);
	}

	if ((*ch) == '.')
	{
		str.push_back('.');
		while ((*(ch = next(ch))) && (*ch) >= '0' && (*ch) <= '9')
		{
			str.push_back(*ch);
		}
	}

	if ((*ch) == 'e' || (*ch) == 'E')
	{
		str.push_back(*ch);
		ch = next(ch);

		if ((*ch) == '-' || (*ch) == '+')
		{
			str.push_back(*ch);
			ch = next(ch);
		}
		while ((*ch) >= '0' && (*ch) <= '9')
		{
			str.push_back(*ch);
			ch = next(ch);
		}
	}

	// Ensure null terminated
	str.push_back('\0');

	float number = 0.0f;

	// Fixme
	sscanf(str.begin(), "%f", &number);

	return number;
}

//-----------------------------------------------------------------------------
bool JSON::parse_bool(const char* s)
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
		default: break;
	}

	CE_ASSERT(false, "Not a boolean");
}

//-----------------------------------------------------------------------------
int32_t JSON::parse_int(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	return (int32_t) parse_number(s);
}

//-----------------------------------------------------------------------------
float JSON::parse_float(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	return (float) parse_number(s);
}

//-----------------------------------------------------------------------------
void JSON::parse_array(const char* s, List<const char*>& array)
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
			array.push_back(ch);

			ch = skip_array(ch);
			ch = skip_object(ch);
			ch = skip_number(ch);
			ch = skip_string(ch);
			ch = skip_bool(ch);

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

	CE_ASSERT(false, "Not an array");
}

//-----------------------------------------------------------------------------
void JSON::parse_object(const char* s, List<JSONPair>& map)
{
	CE_ASSERT_NOT_NULL(s);

	const char* ch = s;

	if ((*ch) == '{')
	{
		ch = next(ch, '{');

		ch = skip_whites(ch);

		if ((*ch) == '}')
		{
			next(ch, '}');
			return;
		}

		while (*ch)
		{
			JSONPair pair;

			pair.key = ch;

			// Skip any value
			ch = skip_array(ch);
			ch = skip_object(ch);
			ch = skip_number(ch);
			ch = skip_string(ch);
			ch = skip_bool(ch);

			ch = skip_whites(ch);
			ch = next(ch, ':');
			ch = skip_whites(ch);

			pair.val = ch;
			map.push_back(pair);

			// Skip any value
			ch = skip_array(ch);
			ch = skip_object(ch);
			ch = skip_number(ch);
			ch = skip_string(ch);
			ch = skip_bool(ch);

			ch = skip_whites(ch);

			if ((*ch) == '}')
			{
				next(ch, '}');
				return;
			}

			ch = next(ch, ',');
			ch = skip_whites(ch);
		}
	}

	CE_ASSERT(false, "Not an object");
}

} // namespace crown
