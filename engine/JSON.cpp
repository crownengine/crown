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

	g_current_char = &str[index];

	if (c && c != current)
	{
		CE_ASSERT(false, "Expected '%c' got '%c'", c, current);
	}

	index++;
	current = str[index];

	return current;
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

	sscanf(str.begin(), "%f", &number);
	printf("\n\nParsed number: %f\n\n", number);
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

		while ((cur = next(s)) != 0)
		{
			array.push_back(g_current_char);
			JSONType t = type(g_current_char);

			while (cur != 0)
			{
				cur = next(s);

				if (t == JT_NUMBER)
				{
					if (cur == ',')
					{
						cur = next(s, ',');
						// Skip whitespace
						while (cur && cur <= ' ')
						{
							cur = next(s);
						}
						break;
					}
				}
				else if (t == JT_ARRAY)
				{
					while (cur && cur != ']')
					{
						cur = next(s);
					}
					while (cur && cur <= ' ')
					{
						cur = next(s);
					}
					if (cur == ',')
					{
						cur = next(s, ' ');
						while (cur && cur <= ' ')
						{
							cur = next(s);
						}
					}
				}

				if (cur == ']')
				{
					cur = next(s, ']');
					return;
				}
			}
		}
	}

	CE_ASSERT(false, "Not an array");
}

} // namespace crown
