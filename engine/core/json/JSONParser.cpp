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

#include "JSONParser.h"
#include "TempAllocator.h"
#include "StringUtils.h"

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

//--------------------------------------------------------------------------
JSONElement::JSONElement() :
	m_parser(NULL),
	m_at(NULL),
	m_begin(NULL)
{
}

//--------------------------------------------------------------------------
JSONElement::JSONElement(JSONParser& parser, const char* at) :
	m_parser(&parser),
	m_at(at),
	m_begin(at)
{
}

//--------------------------------------------------------------------------
JSONElement& JSONElement::operator[](uint32_t i)
{
	TempAllocator1024 alloc;
	List<const char*> array(alloc);

	JSONParser::parse_array(m_at, array);

	CE_ASSERT(i < array.size(), "Index out of bounds");

	m_at = array[i];

	return *this;
}

//--------------------------------------------------------------------------
JSONElement& JSONElement::index(uint32_t i)
{
	return this->operator[](i);
}

//--------------------------------------------------------------------------
JSONElement& JSONElement::key(const char* k)
{
	TempAllocator1024 alloc;
	List<JSONPair> object(alloc);

	JSONParser::parse_object(m_begin, object);

	bool found = false;

	for (uint32_t i = 0; i < object.size(); i++)
	{
		TempAllocator256 key_alloc;
		List<char> key(key_alloc);

		JSONParser::parse_string(object[i].key, key);

		if (string::strcmp(k, key.begin()) == 0)
		{
			m_at = object[i].val;
			found = true;
		}
	}

	CE_ASSERT(found, "Key not found: '%s'", k);

	return *this;
}

//--------------------------------------------------------------------------
bool JSONElement::has_key(const char* k) const
{
	TempAllocator1024 alloc;
	List<JSONPair> object(alloc);
	JSONParser::parse_object(m_begin, object);

	for (uint32_t i = 0; i < object.size(); i++)
	{
		TempAllocator256 key_alloc;
		List<char> key(key_alloc);

		JSONParser::parse_string(object[i].key, key);

		if (string::strcmp(k, key.begin()) == 0)
		{
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
bool JSONElement::is_key_unique(const char* k) const
{
	TempAllocator1024 alloc;
	List<JSONPair> object(alloc);
	JSONParser::parse_object(m_begin, object);

	bool found = false;

	for (uint32_t i = 0; i < object.size(); i++)
	{
		TempAllocator256 key_alloc;
		List<char> key(key_alloc);

		JSONParser::parse_string(object[i].key, key);

		if (string::strcmp(k, key.begin()) == 0)
		{
			if (found == true)
			{
				return false;
			}

			found = true;
		}
	}

	return found;
}

//--------------------------------------------------------------------------
bool JSONElement::bool_value() const
{
	return JSONParser::parse_bool(m_at);
}

//--------------------------------------------------------------------------
int32_t JSONElement::int_value() const
{
	return JSONParser::parse_int(m_at);
}

//--------------------------------------------------------------------------
float JSONElement::float_value() const
{
	return JSONParser::parse_float(m_at);
}

//--------------------------------------------------------------------------
const char* JSONElement::string_value() const
{
	static TempAllocator1024 alloc;
	static List<char> string(alloc);

	string.clear();

	JSONParser::parse_string(m_at, string);

	return string.begin();
}

//--------------------------------------------------------------------------
bool JSONElement::is_nil() const
{
	return JSONParser::type(m_at) == JT_NIL;
}

//--------------------------------------------------------------------------
bool JSONElement::is_bool() const
{
	return JSONParser::type(m_at) == JT_BOOL;
}

//--------------------------------------------------------------------------
bool JSONElement::is_number() const
{
	return JSONParser::type(m_at) == JT_NUMBER;
}

//--------------------------------------------------------------------------
bool JSONElement::is_string() const
{
	return JSONParser::type(m_at) == JT_STRING;
}

//--------------------------------------------------------------------------
bool JSONElement::is_array() const
{
	return JSONParser::type(m_at) == JT_ARRAY;
}

//--------------------------------------------------------------------------
bool JSONElement::is_object() const
{
	return JSONParser::type(m_at) == JT_OBJECT;
}

//--------------------------------------------------------------------------
uint32_t JSONElement::size() const
{
	switch(JSONParser::type(m_at))
	{
		case JT_NIL:
		{
			return 1;
		}
		case JT_OBJECT:
		{
			TempAllocator1024 alloc;
			List<JSONPair> object(alloc);
			JSONParser::parse_object(m_at, object);

			return object.size();
		}
		case JT_ARRAY:
		{
			TempAllocator1024 alloc;
			List<const char*> array(alloc);
			JSONParser::parse_array(m_at, array);

			return array.size();
		}
		case JT_STRING:
		{
			TempAllocator1024 alloc;
			List<char> string(alloc);
			JSONParser::parse_string(m_at, string);

			return string::strlen(string.begin());
		}
		case JT_NUMBER:
		{
			return 1;
		}
		case JT_BOOL:
		{
			return 1;
		}
		default:
		{
			return 0;
		}
	}
}

//--------------------------------------------------------------------------
JSONParser::JSONParser(const char* s) :
	m_document(s)
{
	CE_ASSERT_NOT_NULL(s);
}

//--------------------------------------------------------------------------
JSONElement JSONParser::root()
{
	return JSONElement(*this, skip_whites(m_document));
}

//-----------------------------------------------------------------------------
JSONType JSONParser::type(const char* s)
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
void JSONParser::parse_string(const char* s, List<char>& str)
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

	CE_ASSERT(false, "Bad string");
}

//-----------------------------------------------------------------------------
double JSONParser::parse_number(const char* s)
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

	return string::parse_double(str.begin());
}

//-----------------------------------------------------------------------------
bool JSONParser::parse_bool(const char* s)
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
			CE_ASSERT(false, "Bad boolean");
			return false;
		}
	}
}

//-----------------------------------------------------------------------------
int32_t JSONParser::parse_int(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	return (int32_t) parse_number(s);
}

//-----------------------------------------------------------------------------
float JSONParser::parse_float(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	return (float) parse_number(s);
}

//-----------------------------------------------------------------------------
void JSONParser::parse_array(const char* s, List<const char*>& array)
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

	CE_ASSERT(false, "Bad array");
}

//-----------------------------------------------------------------------------
void JSONParser::parse_object(const char* s, List<JSONPair>& object)
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
			object.push_back(pair);

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

	CE_ASSERT(false, "Bad object");
}

} //namespace crown
