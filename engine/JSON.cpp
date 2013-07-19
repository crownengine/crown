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
#include "Allocator.h"
#include <cstdio>

namespace crown
{

static const char* g_current_char = NULL;

/// This function is EPIC. Love it. Now.
//-----------------------------------------------------------------------------
char JSON::next(const char* str, const char c, bool force_reset)
{
	static char* current_str = (char*)str;
	static size_t index = 0;
	static char current = str[index];

	if (current_str != str || force_reset == true)
	{
		current_str = (char*)str;
		index = 0;
		current = str[index];
	}

	if (c && c != current)
	{
		CE_ASSERT(false, "Expected '%c' got '%c'", c, current);
	}

	index++;
	current = str[index];
	g_current_char = &str[index];

	return current;
}

//-----------------------------------------------------------------------------
char JSON::skip_whites(const char* s)
{
	char cur = (*g_current_char);

	while (cur && cur == ' ') cur = next(s);

	return cur;
}

//-----------------------------------------------------------------------------
char JSON::skip_string(const char* s)
{
	char cur = (*g_current_char);

	bool escaped = false;

	if (cur == '"')
	{
		while ((cur = next(s)) != 0)
		{
			if (cur == '"' && !escaped)
			{
				cur = next(s);
				return cur;
			}
			else if (cur == '\\') escaped = true;
			else escaped = false;
		}
	}

	return cur;
}

//-----------------------------------------------------------------------------
char JSON::skip_number(const char* s)
{
	char cur = (*g_current_char);

	while (cur && ((cur >= '0' && cur <= '9') || cur == '-' || cur == '+' ||
					cur == 'e' || cur == 'E'))  cur = next(s);

	return cur;
}

//-----------------------------------------------------------------------------
char JSON::skip_object(const char* s)
{
	char cur = (*g_current_char);

	uint32_t brackets = 1;

	if (cur == '{')
	{
		brackets++;
		cur = next(s, '{');

		while (cur && brackets != 1)
		{
			if (cur == '}') brackets--;
			else if (cur == '{') brackets++;
			cur = next(s);
		}
	}

	return cur;
}

//-----------------------------------------------------------------------------
char JSON::skip_array(const char* s)
{
	char cur = (*g_current_char);

	uint32_t brackets = 1;

	if (cur == '[')
	{
		brackets++;
		cur = next(s, '[');

		while (cur && brackets != 1)
		{
			if (cur == ']') brackets--;
			else if (cur == '[') brackets++;
			cur = next(s);
		}
	}

	return cur;
}

//-----------------------------------------------------------------------------
char JSON::skip_bool(const char* s)
{
	char cur = (*g_current_char);

	switch (cur)
	{
		case 't':
		{
			cur = next(s, 't');
			cur = next(s, 'r');
			cur = next(s, 'u');
			cur = next(s, 'e');
			break;
		}
		case 'f':
		{
			cur = next(s, 'f');
			cur = next(s, 'a');
			cur = next(s, 'l');
			cur = next(s, 's');
			cur = next(s, 'e');
			break;
		}
		default:
		{
			break;
		}
	}

	return cur;
}

//-----------------------------------------------------------------------------
bool JSON::is_escapee(char c)
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
		default: return s[0] >= '0' && s[0] <= '9' ? JT_NUMBER : JT_BOOL;
	}
}


//-----------------------------------------------------------------------------
void JSON::parse_string(const char* s, List<char>& str)
{
	CE_ASSERT_NOT_NULL(s);

	char cur = '\0';

	if (s[0] == '"')
	{
		while ((cur = next(s)) != 0)
		{
			if (cur == '"')
			{
				next(s);
				str.push_back('\0');
				return;
			}
			else if (cur == '\\')
			{
				cur = next(s);

				if (cur == 'u')
				{
					CE_ASSERT(false, "Not supported at the moment");
				}
				else if (is_escapee(cur))
				{
					str.push_back('\\');
					str.push_back(cur);
				}
				else
				{
					break; // to invalid string
				}
			}
			else
			{
				str.push_back(cur);
			}
		}
	}

	CE_ASSERT(false, "Not a valid string");
}

//-----------------------------------------------------------------------------
double JSON::parse_number(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

 	List<char> str(default_allocator());

 	char cur = s[0];

	if (cur == '-')
	{
		str.push_back('-');
		cur = next(s, '-');
	}
	while (cur >= '0' && cur <= '9')
	{
		str.push_back(cur);
		cur = next(s);
	}

	if (cur == '.')
	{
		str.push_back('.');
		while ((cur = next(s)) != 0 && cur >= '0' && cur <= '9')
		{
			str.push_back(cur);
		}
	}

	if (cur == 'e' || cur == 'E')
	{
		str.push_back(cur);
		cur = next(s);
		if (cur == '-' || cur == '+')
		{
			str.push_back(cur);
			cur = next(s);
		}
		while (cur >= '0' && cur <= '9')
		{
			str.push_back(cur);
			cur = next(s);
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

	switch(s[0])
	{
		case 't':
		{
			next(s, 't');
			next(s, 'r');
			next(s, 'u');
			next(s, 'e');
			return true;
		}
		case 'f':
		{
			next(s, 'f');
			next(s, 'a');
			next(s, 'l');
			next(s, 's');			
			next(s, 'e');
			return false;
		}
		default:
		{
			CE_ASSERT(false, "Current token is not a boolean");
		}
	}
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

	char cur = s[0];

	if (cur == '[')
	{
		cur = next(s, '[');

		// Skip whitespaces
		while (cur && cur <= ' ')
		{
			cur = next(s);
		}

		if (cur == ']')
		{
			cur = next(s, ']');
			return;
		}

		while (cur)
		{
			array.push_back(g_current_char);

			cur = skip_array(s);
			cur = skip_object(s);
			cur = skip_number(s);
			cur = skip_string(s);
			cur = skip_bool(s);

			cur = skip_whites(s);

			// Closing bracket (top-most array)
			if (cur == ']')
			{
				cur = next(s, ']');
				return;
			}

			// Skip until next ','
			cur = next(s, ',');

			// Skip whites, eventually
			cur = skip_whites(s);
		}
	}

	return;
	CE_ASSERT(false, "Not an array");
}

} // namespace crown
