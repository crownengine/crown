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

/// Parses JSON documents.
class JSONParser
{
public:

	/// Read the JSON document contained in the non-null @a s string.
	/// @note
	/// The @a s string has to remain valid for the whole parser's
	/// existence scope.
						JSONParser(const char* s);

	/// Returns the root element of the JSON document.
	JSONParser&			root();

	/// Returns the @a i -th item of the current array.
	JSONParser&			operator[](uint32_t i);

	/// @copydoc JSONParser::operator[]
	JSONParser&			index(uint32_t i);

	/// Returns the element corresponding to key @a k of the
	/// current object.
	/// @note
	/// If the key is not unique in the object scope, the last
	/// key in order of appearance will be selected.
	JSONParser&			key(const char* k);

	/// Returns true wheter the current element is the JSON nil special value.
	bool				is_nil() const;

	/// Returns true wheter the current element is a JSON boolean (true or false).
	bool				is_bool() const;

	/// Returns true wheter the current element is a JSON number.
	bool				is_number() const;

	/// Returns true whether the current element is a JSON string.
	bool				is_string() const;

	/// Returns true whether the current element is a JSON array.
	bool				is_array() const;

	/// Returns true whether the current element is a JSON object.
	bool				is_object() const;

	/// Returns the size of the current element based on the
	/// element's type:
	/// * nil, bool, number: 1
	/// * string: length of the string
	/// * array: number of elements in the array
	/// * object: number of keys in the object
	uint32_t			size() const;

	/// Returns the boolean value of the current element.
	bool				bool_value() const;

	/// Returns the integer value of the current element.
	int32_t				int_value() const;

	/// Returns the float value of the current element.
	float				float_value() const;

	/// Returns the string value of the current element.
	/// @warning
	/// The returned string is kept internally until the next call to
	/// this function, so it is highly unsafe to just keep the pointer
	/// instead of copying its content somewhere else.
	const char*			string_value() const;

public:

	/// Returns the type of the @a s JSON text. 
	static JSONType		type(const char* s);

	/// Parses the @a s JSON string a puts its C representation into @a str.
	static void			parse_string(const char* s, List<char>& str);

	/// Returns the value of the @a s JSON number as double.
	static double		parse_number(const char* s);

	/// Returns the value of the @a s JSON boolean.
	static bool			parse_bool(const char* s);

	/// Returns the value of the @a s JSON number as signed integer.
	static int32_t 		parse_int(const char* s);

	/// Returns the value of the @a s JSON number as float.
	static float		parse_float(const char* s);

	/// Parses the @a s JSON array and puts it into @a array as pointers to
	/// the corresponding items into the original @a s string.
	static void			parse_array(const char* s, List<const char*>& array);

	/// Parses the @a s JSON object and puts it into @a object as pointers to
	/// the corresponding key/value pairs into the original @a s string.
	static void			parse_object(const char* s, List<JSONPair>& object);

private:

	const char* const	m_document;
	const char*			m_at;

private:

	// Disable copying
	JSONParser&			JSONParser(const JSONParser&);
	JSONParser&			operator=(const JSONParser&);
};

} // namespace crown