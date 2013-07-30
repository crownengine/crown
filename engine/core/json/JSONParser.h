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

#pragma once

#include "Types.h"
#include "List.h"

namespace crown
{

enum JSONType
{
	JT_NIL,
	JT_OBJECT,
	JT_ARRAY,
	JT_STRING,
	JT_NUMBER,
	JT_BOOL
};

struct JSONPair
{
	const char* key;
	const char* val;
};

class JSONParser
{
public:

						JSONParser(const char* s);

	JSONParser&			root();

	JSONParser&			operator[](uint32_t i);
	JSONParser&			key(const char* k);

	bool				is_nil() const;
	bool				is_bool() const;
	bool				is_number() const;
	bool				is_string() const;
	bool				is_array() const;
	bool				is_object() const;

	uint32_t			size();

	bool				bool_value() const;
	int32_t				int_value() const;
	float				float_value() const;

public:

	static JSONType		type(const char* s);

	static void			parse_string(const char* s, List<char>& str);
	static double		parse_number(const char* s);

	static bool			parse_bool(const char* s);
	static int32_t 		parse_int(const char* s);
	static float		parse_float(const char* s);

	static void			parse_array(const char* s, List<const char*>& array);
	static void			parse_object(const char* s, List<JSONPair>& map);

private:

	const char* const	m_document;
	const char*			m_at;
};

} // namespace crown