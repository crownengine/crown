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

class JSON
{
public:
						JSON();

	void 				skip_whitespace();

	static char			next(const char* s, const char c = 0, bool force_reset = false);

	static bool			is_escapee(char c);

	static JSONType		type(const char* s);

	static void			parse_string(const char* s, List<char>& str);
	static double		parse_number(const char* s);

	static bool			parse_bool(const char* s);
	static int32_t 		parse_int(const char* s);
	static float		parse_float(const char* s);

	static void			parse_array(const char* s, List<const char*>& array);


	// static void			parse_array(const char* token, List<const char*>& array);
	// static void			parse_object(const char* token, Dictionary<const char* key, const char* val>& dict);

private:

	char			m_buffer[1024];

	uint32_t		m_index;

	char			m_current;
};

} // namespace crown