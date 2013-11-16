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

class JSONParser;

/// Represents a JSON element.
/// The objects of this class are valid until the parser
/// which has generated them, will exist.
class JSONElement
{
public:

	/// Construct the nil JSONElement.
	/// Used to forward-instantiate elements or as a special
	/// nil element.
						JSONElement();
	explicit			JSONElement(const char* at);
						JSONElement(const JSONElement& other);

	JSONElement&		operator=(const JSONElement& other);

	/// Returns the @a i -th item of the current array.
	JSONElement			operator[](uint32_t i);

	/// @copydoc JSONElement::operator[]
	JSONElement			index(uint32_t i);

	/// Returns the @a i -th item of the current array or
	/// the special nil JSONElement() if the index does not exist.
	JSONElement			index_or_nil(uint32_t i);

	/// Returns the element corresponding to key @a k of the
	/// current object.
	/// @note
	/// If the key is not unique in the object scope, the last
	/// key in order of appearance will be selected.
	JSONElement			key(const char* k);

	/// Returns the element corresponding to key @a k of the current
	/// object, or the special nil JSONElement() if the key does not exist.
	JSONElement			key_or_nil(const char* k);

	/// Returns whether the element has the @a k key.
	bool				has_key(const char* k) const;

	/// Returns whether the @a k key is unique in the object
	/// element. If no such key is found it returns false.
	bool				is_key_unique(const char* k) const;

	/// Returns true wheter the element is the JSON nil special value.
	bool				is_nil() const;

	/// Returns true wheter the element is a JSON boolean (true or false).
	bool				is_bool() const;

	/// Returns true wheter the element is a JSON number.
	bool				is_number() const;

	/// Returns true whether the element is a JSON string.
	bool				is_string() const;

	/// Returns true whether the element is a JSON array.
	bool				is_array() const;

	/// Returns true whether the element is a JSON object.
	bool				is_object() const;

	/// Returns the size of the element based on the
	/// element's type:
	/// * nil, bool, number: 1
	/// * string: length of the string
	/// * array: number of elements in the array
	/// * object: number of keys in the object
	uint32_t			size() const;

	/// Returns the boolean value of the element.
	bool				bool_value() const;

	/// Returns the integer value of the element.
	int32_t				int_value() const;

	/// Returns the float value of the element.
	float				float_value() const;

	/// Returns the string value of the element.
	/// @warning
	/// The returned string is kept internally until the next call to
	/// this function, so it is highly unsafe to just keep the pointer
	/// instead of copying its content somewhere else.
	const char*			string_value() const;

	/// Returns the array value of the element.
	/// @note
	/// Calling this function is way faster than accessing individual
	/// array elements by JSONElement::operator[] and it is the very preferred way
	/// for retrieving array elemets. However, you have to be sure that the array
	/// contains only items of the given @array type.
	void				array_value(List<bool>& array) const;

	/// @copydoc JSONElement::array_value(List<bool>&)
	void				array_value(List<int16_t>& array) const;

	/// @copydoc JSONElement::array_value(List<bool>&)
	void				array_value(List<uint16_t>& array) const;

	/// @copydoc JSONElement::array_value(List<bool>&)
	void				array_value(List<int32_t>& array) const;

	/// @copydoc JSONElement::array_value(List<bool>&)
	void				array_value(List<uint32_t>& array) const;

	/// @copydoc JSONElement::array_value(List<bool>&)
	void				array_value(List<float>& array) const;

private:

	const char*			m_at;

	friend class 		JSONParser;
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
	JSONElement			root();

private:

	const char* const	m_document;

private:

	// Disable copying
						JSONParser(const JSONParser&);
	JSONParser&			operator=(const JSONParser&);
};

} // namespace crown